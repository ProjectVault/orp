# orp-xtsfs
FUSE layer for ENCRYPT/DECRYPT XTS filesystem

## Getting Started

This program creates a FUSE layer between the ORP device running an AES-XTS
encryption application, and the operating system.  When the software runs,
it mounts a new filesystem containing two folders, ENCRYPT and DECRYPT.  
Placing a file in the ENCRYPT folder passes it to the ORP device for encryption;
when encryption is complete, the resulting file appears in the DECRYPT folder.
The process can be reverted by placing an encrypted file in the DECRYPT folder,
which will summarily be unencrypted by the ORP device and placed in the ENCRYPT
folder.

## Compilation and Running

To compile, simply run `make`.  This will create a `orpfs` binary executable in the
current directory.

To mount the ORP filesystem, first mount your ORP device, say, at `/mnt/orpdev`:

    # mount -o nosuid,uid=1000,gid=1000,sync /dev/sde1 /mnt/orpdev
    # ls /mnt/orpdev
    RFILE* WFILE*

The ORP device needs to be running the `orp-xts-encryptor` application.  You should
currently just see a solid red light on the device.

Next, run the following command to create the ORP filesystem at, say, `/mnt/orpfs`:

    > ./orpfs [fuse options] MY_SECRET_KEY /mnt/orpdev /mnt/orpfs

At this point, some output will appear, and the yellow light on your device should
start blinking.  This means that the encryption application on the device has been
started successfully (if there is any problem starting the application, an error
should appear on the console).  Now, notice that the ORP filesystem is in place:

    > ls /mnt/orpfs
    DECRYPT/  ENCRYPT/
    > ls /mnt/orpfs/ENCRYPT
    > ls /mnt/orpfs/DECRYPT

Finally, copy a file into `/mnt/orpfs/ENCRYPT` to have it encrypted:

    > cat asdf
    asdfasdfasdf
    asdfasdfasdf
    asdfasdfasdf
    > cp asdf /mnt/orpfs/ENCRYPT/.
    > ls /mnt/orpfs/ENCRYPT
    asdf

Depending on the size of the file, this may take some time for the encryption to
complete.  Once the command has returned, you will notice that a file with the same 
name has now appeared in `/mnt/orpfs/DECRYPT`:

    > ls /mnt/orpfs/DECRYPT
    asdf
    > cat /mnt/orpfs/DECRYPT
    [gibberish]
    > mv /mnt/orpfs/DECRYPT/asdf ./asdf.ciphertext
    > ls /mnt/orpfs/DECRYPT

Also notice that moving the file out of `DECRYPT` has removed the corresponding plaintext
file from the `ENCRYPT` folder.

    ls /mnt/orpfs/ENCRYPT

Finally, you can obtain the unencrypted version by simply placing the file back in the
decrypt folder (or, you can mail it to someone else with a similar ORP device, who can
obtain the unencrypted version:

    > cp ./asdf.ciphertext /mnt/orpfs/DECRYPT/.
    > cat /mnt/orpfs/ENCRYPT/asdf.ciphertext
    asdfasdfasdf
    asdfasdfasdf
    asdfasdfasdf

That's it!  When you're all done, you can unmount everything:

    > fusermount -u /mnt/orpfs
    # umount /mnt/orpdev

## Technical details

1. `orpfs` accepts the standard FUSE command line options.  These must go before the encryption
key on the command line.  Two options that may be of particular use

   * `-d` Mount the ORP FS in debug mode; in particular, this will print out a significant
       amount of output that may allow you to see what calls are being made and if/where
       something is failing.  Moreover, this option does not send `orpfs` into the background,
       enabling debug with `gdb`.  When using this flag, you can unmount the filesystem
       with `fusermount` as above, or with `Ctrl-C`
   * `-s` Run the FUSE layer in single-threaded mode.  In this mode, the filesystem may be less
       responsive, as it will not handle other operations (such as browsing or viewing files)
       while an encryption or decryption is being performed.  My tests indicate that under
       normal usage, this option shouldn't be necessary, HOWEVER: none of the data structures
       in ORP FS are thread-safe.  In particular, I would expect things to break rapidly if
       you perform multiple writes to the filesystem at the same time.
       
2. I have observed some issues with timing misbehavior in tests.  In particular, it is necessary
to do a sleep after performing an RFILE acknolwedgement to prevent dropped interrupts on the 
ORP device.  I have fine-tuned this delay to a safe value for my system.  However, it may be
worth tweaking this sleep value up or down if you experience problems in a different environment.
