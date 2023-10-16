# cypher
toy invertable file "encryption" using SHA256 hashes

```  
usage:
    ./cypher [options] <inpath> <key>
arguments:
    inpath
        input filepath
    key
        key used to encrypt file data
options:
    --help
    -h
        print application usage
        TYPE: FLAG
        DEFAULT: true
    --bufsize
    -b
        set buffer size for file io
        TYPE: PARAMETER
        DEFAULT: 1028
    --filekey
    -f
        read key from file (key argument is path)
        TYPE: FLAG
    --outpath
    -o
        output filepath
        TYPE: PARAMETER
        DEFAULT: out.enc
```