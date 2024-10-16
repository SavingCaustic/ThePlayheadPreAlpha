Error handling is used to avoid standard std::cout and to be able to catch errors to log etc.
Architecture

[AudioErrorBuffer] => [AudioErrorProxy] => [ErrorBuffer] => [ErrorReader]

Proxy and Reader are threads controlled by [ErrorHandler]
Only writing to ErrorBuffer is thread safe
