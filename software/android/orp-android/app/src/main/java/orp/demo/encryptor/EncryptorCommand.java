/**
 * @file EncryptorCommand.java
 */

package orp.demo.encryptor;

import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

import orp.orp.TIDL;

public enum EncryptorCommand
{
  EC_DATA (64),
  EC_DONE (80);
  private final int value;
  
  private EncryptorCommand(int in_value)
  {
    value = in_value;
  }
  
  public int getValue()
  {
    return value;
  }
  
  public static void Serialize(ByteArrayOutputStream out, EncryptorCommand in) throws TIDL.TIDLException
  {
    switch (in)
    {
      case EC_DATA:
        TIDL.int32_serialize(out, 64);
        break;
      case EC_DONE:
        TIDL.int32_serialize(out, 80);
        break;
    }
  }
  
  public static EncryptorCommand Deserialize(ByteBuffer in) throws TIDL.TIDLException
  {
    EncryptorCommand ret;
    int v = TIDL.int32_deserialize(in);
    
    switch (v)
    {
      case 64:
        ret = EncryptorCommand.EC_DATA;
        break;
      case 80:
        ret = EncryptorCommand.EC_DONE;
        break;
      default: throw new TIDL.UnrecognizedEnumException(v);
    }
    
    return ret;
  }
}
