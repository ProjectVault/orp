/**
 * @file GCM_Mode.java
 */

package orp.demo.bulkencryption;

import orp.orp.TIDL;
import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

public enum GCM_Mode
{
  GCM128_ENCRYPT (16),
  GCM128_DECRYPT (32);
  private final int value;
  
  private GCM_Mode(int in_value)
  {
    value = in_value;
  }
  
  public int getValue()
  {
    return value;
  }
  
  public static void Serialize(ByteArrayOutputStream out, GCM_Mode in) throws TIDL.TIDLException
  {
    switch (in)
    {
      case GCM128_ENCRYPT:
        TIDL.int32_serialize(out, 16);
        break;
      case GCM128_DECRYPT:
        TIDL.int32_serialize(out, 32);
        break;
    }
  }
  
  public static GCM_Mode Deserialize(ByteBuffer in) throws TIDL.TIDLException
  {
    GCM_Mode ret;
    int v = TIDL.int32_deserialize(in);
    
    switch (v)
    {
      case 16:
        ret = GCM_Mode.GCM128_ENCRYPT;
        break;
      case 32:
        ret = GCM_Mode.GCM128_DECRYPT;
        break;
      default: throw new TIDL.UnrecognizedEnumException(v);
    }
    
    return ret;
  }
}
