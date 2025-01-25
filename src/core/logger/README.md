Logger handling is used to avoid standard std::cout and to be able to catch errors to log etc.
Architecture

[AudioLoggerBuffer] => [AudioLoggerProxy] => [LoggerBuffer] => [LoggerReader]

Proxy and Reader are threads controlled by [LoggerHandler]
Only writing to LoggerBuffer is thread safe
