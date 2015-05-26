/**
 * @file EncryptorAlgo.java
 */

package orp.demo.encryptor;

import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

import orp.orp.TIDL;

public enum EncryptorAlgo
{
  EC_GCM_128 (0),
  EC_GCM_192 (1),
  EC_GCM_256 (2);
  private final int value;
  
  private EncryptorAlgo(int in_value)
  {
    value = in_value;
  }
  
  public int getValue()
  {
    return value;
  }
  
  public static void Serialize(ByteArrayOutputStream out, EncryptorAlgo in) throws TIDL.TIDLException
  {
    switch (in)
    {
      case EC_GCM_128:
        TIDL.int32_serialize(out, 0);
        break;
      case EC_GCM_192:
        TIDL.int32_serialize(out, 1);
        break;
      case EC_GCM_256:
        TIDL.int32_serialize(out, 2);
        break;
    }
  }
  
  public static EncryptorAlgo Deserialize(ByteBuffer in) throws TIDL.TIDLException
  {
    EncryptorAlgo ret;
    int v = TIDL.int32_deserialize(in);
    
    switch (v)
    {
      case 0:
        ret = EncryptorAlgo.EC_GCM_128;
        break;
      case 1:
        ret = EncryptorAlgo.EC_GCM_192;
        break;
      case 2:
        ret = EncryptorAlgo.EC_GCM_256;
        break;
      default: throw new TIDL.UnrecognizedEnumException(v);
    }
    
    return ret;
  }
}
