/**
 * @file EncryptorResponse.java
 */

package orp.demo.encryptor;

import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

import orp.orp.TIDL;

public enum EncryptorResponse
{
  ER_ERROR (255),
  ER_UNSUPPORTED (254),
  ER_OK (1);
  private final int value;
  
  private EncryptorResponse(int in_value)
  {
    value = in_value;
  }
  
  public int getValue()
  {
    return value;
  }
  
  public static void Serialize(ByteArrayOutputStream out, EncryptorResponse in) throws TIDL.TIDLException
  {
    switch (in)
    {
      case ER_ERROR:
        TIDL.int32_serialize(out, 255);
        break;
      case ER_UNSUPPORTED:
        TIDL.int32_serialize(out, 254);
        break;
      case ER_OK:
        TIDL.int32_serialize(out, 1);
        break;
    }
  }
  
  public static EncryptorResponse Deserialize(ByteBuffer in) throws TIDL.TIDLException
  {
    EncryptorResponse ret;
    int v = TIDL.int32_deserialize(in);
    
    switch (v)
    {
      case 255:
        ret = EncryptorResponse.ER_ERROR;
        break;
      case 254:
        ret = EncryptorResponse.ER_UNSUPPORTED;
        break;
      case 1:
        ret = EncryptorResponse.ER_OK;
        break;
      default: throw new TIDL.UnrecognizedEnumException(v);
    }
    
    return ret;
  }
}
