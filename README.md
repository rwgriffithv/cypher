# cypher
toy invertable file "encryption" using SHA256 hashes

```
usage:
    cypher <key> [options]
arguments:
    key
        key used to pseudo-encrypt input bytes
options:
    --help
        -h
        print application usage (to stderr)
    --sha256
        -s
        output SHA256 hash of key (ignore input)
    --bufsize <bytes>
        -b <bytes>
        (default: 2056)
        set buffer size for file io in bytes
    --keyfile
        -k
        use bytes of file at <key> argument as key
    --logfile <path>
        -l <path>
        log verbose info to filepath
    --infile <path>
        -i <path>
        read input from filepath (instead of stdin)
    --outfile <path>
        -o <path>
        write output to filepath (instead of stdout)
```