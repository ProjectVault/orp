# orp-encryptor

The ORP stream encryptor application uses AES-GCM mode to encrypt and decrypt 
blobs of data.  An encryption key can be provided, which is mixed with a private
application key and used to encrypt or decrypt the data.  The private application key is
never revealed to the user of the application.

## Installation and Usage

Configure the application with the following command:

    ./configure --prefix=/path/to/install CFLAGS=-mboard=or1ksim-uart --host=or1k-elf

Then, create the binary using `make; make install`.  This will generate a binary
called `bundle-encryptor`, which can be loaded onto the ORP device following the
instructions in the ORP repository.

Communication with the application is managed via TIDL.  All TIDL enums and commands
are given in `encproto.tidl`.  At a high level, the steps needed to use the encryptor 
application are the following:

1.  Start the application by sending a message to the ORP session manager
2.  Initialize the application key and mode of operation
3.  Stream data to the application to be encrypted or decrypted
4.  End the encryption stream; go to step 2 or shut down the application

Each of these steps is described in detail below.

### Start the application

The endpoint for the ORP encryption application is a 32-byte string.  The first four
bytes are `0x65 0x6e 0x63 0xa`; all remaining bytes in the endpoint are `0x0`.

### Initialize the application

Once the application has been started, the first message must set the the mode (encrypt, 
decrypt, or shutdown), the encryption algorithm (128-, 192-, or 256-bit), and the
user-supplied key.  The specific format of this first message is

    [mode|algo|key]

where `mode` is one of `EC_ENCRYPT`, `EC_DECRYPT`, or `EC_SHUTDOWN`, `algo` is one of
`EC_GCM_128`, `EC_GCM_192`, or `EC_GCM_256` (currently, only `EC_GCM_128` is supported by
the application).  The key is an arbitrary-length string (up to the size of `WFILE`) which 
is mixed in with the application keys.

The ORP device will provide a response indicating the success or failure of the initialization.
`EC_OK` means the application is ready to process data; `EC_ERROR` means there was an
error parsing the input message; `EC_UNSUPPORTED` means the chosen algorithm is not supported.

### Stream data to the application

Once the application has been initialized, arbitrary data can be streamed to the application.
The format of these messages is

    [cmd|data]

where `cmd` is one of `EC_DATA` or `EC_DONE`.  If `cmd` is `EC_DATA`, the remainder of the
message is either encrypted or decrypted, according to the mode set during initialization.
In this case, the application will respond with `EC_OK`, followed by the encrypted data.
If there were any problems doing the encryption or parsing the input message, the application 
will respond with `EC_ERROR`.

### End the encryption stream

Otherwise, if `cmd` is `EC_DONE`, the application will return to the initial state, and 
respond with `EC_OK`.  At this stage, the application requires a new encryption key,
mode, and algorithm before any more data can be processed.  Alternately, if no more data
is to be processed, the application can be shutdown by sending `EC_SHUTDOWN`
