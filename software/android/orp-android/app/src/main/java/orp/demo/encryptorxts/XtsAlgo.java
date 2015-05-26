/**
 * @file XtsAlgo.java
 */

package orp.demo.encryptorxts;

import orp.orp.TIDL;
import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

public enum XtsAlgo
{
  XTS_128 (0),
  XTS_256 (2);
  private final int value;
  
  private XtsAlgo(int in_value)
  {
    value = in_value;
  }
  
  public int getValue()
  {
    return value;
  }
  
  public static void Serialize(ByteArrayOutputStream out, XtsAlgo in) throws TIDL.TIDLException
  {
    switch (in)
    {
      case XTS_128:
        TIDL.int32_serialize(out, 0);
        break;
      case XTS_256:
        TIDL.int32_serialize(out, 2);
        break;
    }
  }
  
  public static XtsAlgo Deserialize(ByteBuffer in) throws TIDL.TIDLException
  {
    XtsAlgo ret;
    int v = TIDL.int32_deserialize(in);
    
    switch (v)
    {
      case 0:
        ret = XtsAlgo.XTS_128;
        break;
      case 2:
        ret = XtsAlgo.XTS_256;
        break;
      default: throw new TIDL.UnrecognizedEnumException(v);
    }
    
    return ret;
  }
}
