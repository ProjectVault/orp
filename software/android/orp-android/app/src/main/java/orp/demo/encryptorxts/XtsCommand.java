/**
 * @file XtsCommand.java
 */

package orp.demo.encryptorxts;

import orp.orp.TIDL;
import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

public enum XtsCommand
{
  XTS_ENCRYPT (16),
  XTS_DECRYPT (32),
  XTS_SHUTDOWN (48);
  private final int value;
  
  private XtsCommand(int in_value)
  {
    value = in_value;
  }
  
  public int getValue()
  {
    return value;
  }
  
  public static void Serialize(ByteArrayOutputStream out, XtsCommand in) throws TIDL.TIDLException
  {
    switch (in)
    {
      case XTS_ENCRYPT:
        TIDL.int32_serialize(out, 16);
        break;
      case XTS_DECRYPT:
        TIDL.int32_serialize(out, 32);
        break;
      case XTS_SHUTDOWN:
        TIDL.int32_serialize(out, 48);
        break;
    }
  }
  
  public static XtsCommand Deserialize(ByteBuffer in) throws TIDL.TIDLException
  {
    XtsCommand ret;
    int v = TIDL.int32_deserialize(in);
    
    switch (v)
    {
      case 16:
        ret = XtsCommand.XTS_ENCRYPT;
        break;
      case 32:
        ret = XtsCommand.XTS_DECRYPT;
        break;
      case 48:
        ret = XtsCommand.XTS_SHUTDOWN;
        break;
      default: throw new TIDL.UnrecognizedEnumException(v);
    }
    
    return ret;
  }
}
