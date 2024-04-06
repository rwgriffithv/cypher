# cypher
toy invertable file "encryption" using SHA256 hashes

```
usage:
    cypher [options] <inpath> <key>
arguments:
    inpath
        input filepath
    key
        key used to encrypt file data
options:
    --help
        -h
        print application usage
    --bufsize <bytes>
        -b <bytes>
        (default: 1028)
        set buffer size for file io in bytes
    --keyfile
        -k
        treat <key> argument as file to read key from
    --outpath <path>
        -o <path>
        (default: out.enc)
        output filepath
```