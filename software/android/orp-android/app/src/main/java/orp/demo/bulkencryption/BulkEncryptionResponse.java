/**
 * @file BulkEncryptionResponse.java
 */

package orp.demo.bulkencryption;

import orp.orp.TIDL;
import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

public enum BulkEncryptionResponse
{
  BER_ERROR (255),
  BER_GOODBYE (17),
  BER_DHPAIR (33),
  BER_DH_EXCHANGED (49),
  BER_GCM_READY (81),
  BER_GCM_BLOCKS (97);
  private final int value;
  
  private BulkEncryptionResponse(int in_value)
  {
    value = in_value;
  }
  
  public int getValue()
  {
    return value;
  }
  
  public static void Serialize(ByteArrayOutputStream out, BulkEncryptionResponse in) throws TIDL.TIDLException
  {
    switch (in)
    {
      case BER_ERROR:
        TIDL.int32_serialize(out, 255);
        break;
      case BER_GOODBYE:
        TIDL.int32_serialize(out, 17);
        break;
      case BER_DHPAIR:
        TIDL.int32_serialize(out, 33);
        break;
      case BER_DH_EXCHANGED:
        TIDL.int32_serialize(out, 49);
        break;
      case BER_GCM_READY:
        TIDL.int32_serialize(out, 81);
        break;
      case BER_GCM_BLOCKS:
        TIDL.int32_serialize(out, 97);
        break;
    }
  }
  
  public static BulkEncryptionResponse Deserialize(ByteBuffer in) throws TIDL.TIDLException
  {
    BulkEncryptionResponse ret;
    int v = TIDL.int32_deserialize(in);
    
    switch (v)
    {
      case 255:
        ret = BulkEncryptionResponse.BER_ERROR;
        break;
      case 17:
        ret = BulkEncryptionResponse.BER_GOODBYE;
        break;
      case 33:
        ret = BulkEncryptionResponse.BER_DHPAIR;
        break;
      case 49:
        ret = BulkEncryptionResponse.BER_DH_EXCHANGED;
        break;
      case 81:
        ret = BulkEncryptionResponse.BER_GCM_READY;
        break;
      case 97:
        ret = BulkEncryptionResponse.BER_GCM_BLOCKS;
        break;
      default: throw new TIDL.UnrecognizedEnumException(v);
    }
    
    return ret;
  }
}
