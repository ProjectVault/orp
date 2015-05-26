/**
 * @file XtsResponse.java
 */

package orp.demo.encryptorxts;

import orp.orp.TIDL;
import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

public enum XtsResponse
{
  XTS_ERROR (255),
  XTS_UNSUPPORTED (254),
  XTS_OK (1);
  private final int value;
  
  private XtsResponse(int in_value)
  {
    value = in_value;
  }
  
  public int getValue()
  {
    return value;
  }
  
  public static void Serialize(ByteArrayOutputStream out, XtsResponse in) throws TIDL.TIDLException
  {
    switch (in)
    {
      case XTS_ERROR:
        TIDL.int32_serialize(out, 255);
        break;
      case XTS_UNSUPPORTED:
        TIDL.int32_serialize(out, 254);
        break;
      case XTS_OK:
        TIDL.int32_serialize(out, 1);
        break;
    }
  }
  
  public static XtsResponse Deserialize(ByteBuffer in) throws TIDL.TIDLException
  {
    XtsResponse ret;
    int v = TIDL.int32_deserialize(in);
    
    switch (v)
    {
      case 255:
        ret = XtsResponse.XTS_ERROR;
        break;
      case 254:
        ret = XtsResponse.XTS_UNSUPPORTED;
        break;
      case 1:
        ret = XtsResponse.XTS_OK;
        break;
      default: throw new TIDL.UnrecognizedEnumException(v);
    }
    
    return ret;
  }
}
