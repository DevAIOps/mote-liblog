一个非常简单、独立、可高度定制的日志库，支持简单的日志打印、级别设置、日志切割等功能，同时针对线程、进程编程进行了简单的优化，只依赖基础的 `glibc` 和 `libase` 库，没有其它的依赖。

希望可以通过简单的定制可以快速适配不同的场景，所以，很多的配置都是通过宏定义在编译阶段已经确定了。

# 1. 简介

支持如下的特性：

* 针对不同场景进行优化，支持标准输出、单进程、多进程、多线程几种方式，并进行了优化。
* 支持日志级别动态修改，可以通过类似信号等机制进行调整，例如查看更详细日志。
* 文件自动切割，可以设置文件大小、保留多少个文件。
* 打印二进制，其输出格式与 `xxd` 命令类似，接口的前缀使用 `logh_XXX` 。

在 CMake 中，可以通过如下方式进行配置。

```
ADD_DEFINITIONS(-DLOG_USE_FILENO=1)  # 所有，包括库，优先级低
TARGET_COMPILE_OPTIONS(${PROJECT_NAME}Ctl PRIVATE "-DLOG_USE_FILENO=1" "-DXXXX")  # 单个，优先级高
```
## 1.1 实现介绍

为了尽量减少对函数的调用，将对应的 `log_XXX()` 接口都是通过宏来实现，这样，当日志级别较小时，会先进行判断，如果不满足条件，那么就不会再调用打印日志函数，可以适当优化性能。

不过上述的优化，也同时意味着这些函数无法以函数参数方式传递，所以，单独实现了一个 `log_out()` 函数。

## 1.2 日志级别

包含了如下几种。

```
LOG_PANIC    最严重的错误，程序无法正常运行，会打印日志并直接退出
LOG_FATAL    严重错误，会导致部分功能不可用，但是基础功能可用
LOG_ERROR    错误，由于业务逻辑异常导致，如参数校验不通过，会影响任务的正常执行
LOG_WARNING  告警，一些可能的异常信息，但不影响业务运行
LOG_NOTICE   提示信息，例如使用默认参数
LOG_INFO     信息，业务执行逻辑信息
LOG_DEBUG    调试信息，通常用于给OPS角色排查问题使用
LOG_TRACE    详细调试信息，一般用于开发定位一些异常信息
```

除了上述的日志之外，还有一种审计日志，通过 `log_audit()` 输出，此时不再受日志级别的限制，只要磁盘空间足够就会打印。

# 2. API

## 2.1 初始化

一般启动时需要通过 `log_init()` 进行初始化，此时需要检查是否返回值，如果小于 0 则表示日志打开失败，会向标准错误输出具体原因，通常是由于路径或者权限问题导致。

```c
int log_init(const char *file, int level, int flag);
```

其中 `file` 在需要输出到文件时有用，否则虽然可以设置，但是无效；`level` 指定默认的日志级别，如上所述；而 `flag` 指定了使用的模式，介绍如下。

* `LOG_F_STDOUT` (默认) `LOG_F_STDERR` 打印到标准输出或者标准错误输出，无需打开文件。
* `LOG_F_PROC` 单进程方式打开，会一直持有文件句柄，直到进程退出。
* `LOG_F_MULTI` 多进程方式打开，每次打印日志时候会打开文件，写入完成后关闭，从而满足原子性。
* `LOG_F_THREAD` 多线程方式打开，同样会一直持有文件句柄，同时会根据线程锁防止竞争，而且每个线程包含线程级的缓存，以提高性能。

## 2.2 打印日志

可以按照所需打印日志即可。

```
----- 日志打印，按照日志级别由低到高，可以将log开头替换为logh打印二进制
log_audit(...)
log_panic(...)
log_fatal(...)
log_error(...)
log_warning(...)
log_notice(...)
log_info(...)
log_debug(...)
log_trace(...)

int log_level_inc(void); /* less logs */
int log_level_dec(void); /* more logs */
int log_set_level(int level);
int log_get_level(const char *level);
const char *log_get_name(const int level);
const char *log_get_mode(void);
```

## 2.3 配置

为了保证简单，这里大部分的配置都是以宏的方式提供，例如缓存大小、日至切割等，用户可以编译的时候指定。

```
----- 是否支持线程
#define LOG_USE_THREAD          0

----- 是否使用文件名+行号打印日志
#define LOG_USE_FILENO          0

----- 日志文件的最大，以及保留日志文件数
#define LOG_FILE_SIZE_MAX       1024 * 1024 * 50
#define LOG_FILE_NUMS_MAX       4

----- 缓存的最大最小值
#define LOG_BUFFER_MIN          1024
#define LOG_BUFFER_MAX          16 * 1024

----- 当panic时的退出码
#define LOG_LEAVE_CODE          123

----- 定制文件打开时的权限、标识等
#define LOG_FILE_MODE           (S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH)
#define LOG_FILE_FLAG           (O_CLOEXEC | O_WRONLY | O_APPEND | O_CREAT)

----- 定制备份文件的权限
#define LOG_BAK_FILE_MODE       (S_IRUSR | S_IRGRP | S_IROTH)
```

# 3. 高级进阶

## 3.1 功能调试

通过 `LOG_PROFILE` 和 `LOG_PROFILE_LEVEL` 可以设置调试日志打印文件以及日志级别，会打印常见的报错信息。

## 3.2 打印行号

默认是不会打印文件名以及行号的，可以将 `LOG_USE_FILENO` 设置为 `1` 开启，不过在 CMake 中默认的 `__FILE__` 宏会扩展为绝对路径，所以，这里使用 `__FILENAME__`，然后在 `CMakeLists.txt` 中添加如下内容。

```
# It's relative file path.
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")
# Only file name.
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(notdir $(abspath $<))\"'")
```

其中 GCC 中可以使用的宏还包括了 `__FILE__` `__FUNCTION__` `__LINE__` 等。

## 3.3 自动切换

允许从 STDOUT/STDERR 切换到其它模式，也就是开始从标准输出打印，在读取完配置之后，切换到对应的模式，例如，输出到文件中。

``` c
log_init(NULL, log_get_level(loglevel), LOG_F_STDOUT);
log_init("foobar.log", log_get_level(loglevel), LOG_F_PROC);
```

简单来说，就是如果写入到的是标准输出、标准错误输出，实际上是不会标记为初始化成功的。

# 4. 其它

* 当 `panic` 时，会直接调用 `_exit()` 退出，此时对于像通过 `atexit()` 注册的退出函数不会执行，就可能会出现内存泄漏。
* 在 [Github logrus](https://github.com/Sirupsen/logrus) 中有篇原子写入的文章。


<!--
## 回调

如果要将其作为日志回调函数，可以使用如下方式。

```
void ring_dump(struct ring *ring, void (*log)(const char *format, ...))
{
	if (log_level < LOG_INFO)
		return;
	log(LOG_INFO, "current pointer address %p.", ring);
}
```

因为类似 `log_info` 是宏定义，所以无法直接使用。


日志使用最佳实践
https://dev.to/raysaltrelli/logging-best-practices-obo
https://www.scalyr.com/blog/the-10-commandments-of-logging/
/* compress data through zlib */int zcompress(uint8_t *data, int ndata, uint8_t *buff, int *nbuff)
{
        z_stream stream;
        int rc, outlen = *nbuff;
        if (data == NULL || ndata <= 0)
                return 0;

        memset(&stream, 0, sizeof(stream));
        //rc = deflateInit(&stream, Z_DEFAULT_COMPRESSION);        rc = deflateInit(&stream, 9);
        if (rc != Z_OK) {
                fprintf(stderr, "deflate init failed, rc %d.\n", rc);
                return -1;
        }

        stream.next_in   = data;
        stream.avail_in  = ndata;
        stream.next_out  = buff;
        stream.avail_out = outlen;

        do {
                rc = deflate(&stream, Z_NO_FLUSH);
                if (rc != Z_OK) {
                        fprintf(stderr, "deflate Z_NO_FLUSH failed, rc %d.\n", rc);
                        return -1;
                }
        } while(stream.avail_in > 0 && stream.total_out < outlen);
        if (stream.avail_in > 0)
                return stream.avail_in;
        for (;;) {
                rc = deflate(&stream, Z_FINISH);
                if (rc == Z_STREAM_END)
                        break;
                if (rc != Z_OK) {
                        fprintf(stderr, "deflate Z_FINISH failed, rc %d.\n", rc);
                        return -1;
                }
        }

        rc = deflateEnd(&stream);
        if (rc != Z_OK) {
                fprintf(stderr, "deflate end failed, rc %d.\n", rc);
                return -1;
        }

        *nbuff = stream.total_out;
        return 0;
}
/* uncompress data */
int zdecompress(uint8_t *data, int ndata, uint8_t *buff, int *nbuff)
{
        z_stream stream;
        int len = *nbuff, rc;

        memset(&stream, 0, sizeof(stream));
        rc = inflateInit(&stream);
        if (rc != Z_OK) {
                fprintf(stderr, "inflate init failed, rc %d.\n", rc);
                return -1;
        }

        stream.next_in   = data;
        stream.avail_in  = ndata;
        stream.next_out  = buff;
        stream.avail_out = len;
        do {
                rc = inflate(&stream, Z_NO_FLUSH);
                if (rc == Z_STREAM_END)
                        break;
                if (rc != Z_OK) {
                        fprintf(stderr, "inflate Z_NO_FLUSH failed, rc %d.\n", rc);
                        return -1;
                }
        } while (stream.total_out < len && stream.total_in < ndata);

        rc = inflateEnd(&stream);
        if (rc != Z_OK) {
                fprintf(stderr, "inflate end failed, rc %d.\n", rc);
                return -1;
        }
        *nbuff = stream.total_out;

        return 0;
}
int main(void)
{
        int len, outlen, rc;
        uint8_t buff[4096], plain[4096];
        uint8_t *data = "Answer to the Ultimate Question of Life, the Universe, and Everything.";

        outlen = sizeof(buff);
        rc = zcompress(data, strlen(data) + 1, buff, &outlen);
        if (rc != 0) {
                fprintf(stderr, "compress through zlib method failed, rc %d.\n", rc);
                return -1;
        }
        fprintf(stdout, "original %dB compress to %dB.\n", strlen(data) + 1, outlen);

        len = outlen;
        outlen = sizeof(plain);
        rc = zdecompress(buff, len, plain, &outlen);
        if (rc != 0) {
                fprintf(stderr, "decompress through zlib method failed, rc %d.\n", rc);
                return -1;
        }
        fprintf(stdout, "got data: %s.\n", plain);
}
package main

import (
        "bufio"
        "compress/zlib"
        "io"
        "os"
)

func main() {
        f, err := os.Open("test.txt.zip")
        if err != nil {
                panic(err)
        }
        b := bufio.NewReader(f)

        r, err := zlib.NewReader(b)
        if err != nil {
                panic(err)
        }
        io.Copy(os.Stdout, r)
        r.Close()
}
gzip解压缩
https://blog.csdn.net/aican_yu/article/details/6745878

待支持特性：
* 不同模块输出，也就是将不同的功能模块、日志级别等输出到不同文件中。
* 压缩，支持指定模式的文件压缩。

待验证：
* 内存缩减，这里使用的是一个可以动态伸缩的缓存，很多时候即使释放给 glibc ，实际也并为被系统回收。
-->
