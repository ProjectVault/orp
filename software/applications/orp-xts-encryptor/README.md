# orp-xts-encryptor

The ORP XTS encryptor application uses AES-XTS mode to encrypt and decrypt 
files, filesystems, or filesystem-like data.  An encryption key can be provided, 
which is mixed with a private application key and used to encrypt or decrypt the data.  
The private application key is never revealed to the user of the application.

The XTS mode of AES encrypts data that is logically divided up into "blocks" or "sectors".
Each block has a fixed size, and can be encrypted or decrypted independently from any
other block of data.  To do this, it uses a "sequence number", which is simply the location
of the block of data inside the entire collection (for example, if you are performing
whole-disk or filesystem encryption, the sequence number might correspond to the sector
number of the data).

## Installation and Usage

Configure the application with the following command:

    ./configure --prefix=/path/to/install CFLAGS=-mboard=or1ksim-uart --host=or1k-elf

Then, create the binary using `make; make install`.  This will generate a binary
called `bundle-xts-encryptor`, which can be loaded onto the ORP device following the
instructions in the ORP repository.

Communication with the application is managed via TIDL.  All TIDL enums and commands
are given in `xtsproto.tidl`.  At a high level, the steps needed to use the encryptor 
application are the following:

1.  Start the application by sending a message to the ORP session manager
2.  Initialize the application key and mode of operation
3.  Send blocks of data to be encrypted or decrypted
4.  End the encryption stream; go to step 2

Each of these steps is described in detail below.

### Start the application

The endpoint for the ORP encryption application is a 32-byte string.  The first four
bytes are `0x78 0x74 0x73 0xa`; all remaining bytes in the endpoint are `0x0`.

### Initialize the application

Once the application has been started, the first message must set the the mode (encrypt, 
decrypt, or shutdown), the encryption algorithm (128-, 192-, or 256-bit), and the
user-supplied key.  The specific format of this first message is

    [algo|block-size|key]

where `algo` is one of `XTS_128` or `XTS_256` (currently, only `XTS_128` is supported by 
the application).  The `block-size` parameter is the number of bytes in each block of data 
to be processed; this parameter must be a multiple of the `AES_BLOCK_SIZE`, that is, 16 
bytes).  The key is an arbitrary-length string (up to the size of `WFILE`) which is mixed 
in with the application keys.

The ORP device will provide a response indicating the success or failure of the initialization.
`XTS_OK` means the application is ready to process data; `XTS_ERROR` means there was an
error parsing the input message; `XTS_UNSUPPORTED` means the chosen algorithm is not supported.

### Stream data to the application

Once the application has been initialized, arbitrary data can be streamed to the application.
The format of these messages is

    [cmd|sequence-id|data]

where `cmd` is one of `XTS_ENCRYPT`, `XTS_DECRYPT`, or `XTS_SHUTDOWN`.  The `sequence-id` is
the location of this block of data in the overall collection of data (e.g., the sector number
of the filesystem).  Finally, data must be a number of bytes to encrypt matching the `block-size`
parameter passed in during initialization.

If `cmd` is `XTS_ENCRYPT` or `XTS_DECRYPT`, the data message is either encrypted or decrypted 
with the key set during initialization.  In this case, the application will respond with `EC_OK`, 
followed by the encrypted data.  If there were any problems doing the encryption or parsing the 
input message, the application will respond with `EC_ERROR`.

### End the encryption stream

Otherwise, if `cmd` is `XTS_SHUTDOWN`, the application will return to the initial state, and 
respond with `EC_OK`.  At this stage, the application requires a new encryption key,
block-size, and algorithm before any more data can be processed.  

