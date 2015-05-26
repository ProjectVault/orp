/**
 * @file EncryptorMode.java
 */

package orp.demo.encryptor;

import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

import orp.orp.TIDL;

public enum EncryptorMode
{
  EC_ENCRYPT (16),
  EC_DECRYPT (32),
  EC_SHUTDOWN (48);
  private final int value;
  
  private EncryptorMode(int in_value)
  {
    value = in_value;
  }
  
  public int getValue()
  {
    return value;
  }
  
  public static void Serialize(ByteArrayOutputStream out, EncryptorMode in) throws TIDL.TIDLException
  {
    switch (in)
    {
      case EC_ENCRYPT:
        TIDL.int32_serialize(out, 16);
        break;
      case EC_DECRYPT:
        TIDL.int32_serialize(out, 32);
        break;
      case EC_SHUTDOWN:
        TIDL.int32_serialize(out, 48);
        break;
    }
  }
  
  public static EncryptorMode Deserialize(ByteBuffer in) throws TIDL.TIDLException
  {
    EncryptorMode ret;
    int v = TIDL.int32_deserialize(in);
    
    switch (v)
    {
      case 16:
        ret = EncryptorMode.EC_ENCRYPT;
        break;
      case 32:
        ret = EncryptorMode.EC_DECRYPT;
        break;
      case 48:
        ret = EncryptorMode.EC_SHUTDOWN;
        break;
      default: throw new TIDL.UnrecognizedEnumException(v);
    }
    
    return ret;
  }
}
