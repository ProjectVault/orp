/**
 * @file BulkEncryptionCommand.java
 */

package orp.demo.bulkencryption;

import orp.orp.TIDL;
import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

public enum BulkEncryptionCommand
{
  BEC_LOGOUT (16),
  BEC_GEN_DHPAIR (32),
  BEC_DH_EXCHANGE (48),
  BEC_GCM_INIT (80),
  BEC_GCM_DOBLOCK (96);
  private final int value;
  
  private BulkEncryptionCommand(int in_value)
  {
    value = in_value;
  }
  
  public int getValue()
  {
    return value;
  }
  
  public static void Serialize(ByteArrayOutputStream out, BulkEncryptionCommand in) throws TIDL.TIDLException
  {
    switch (in)
    {
      case BEC_LOGOUT:
        TIDL.int32_serialize(out, 16);
        break;
      case BEC_GEN_DHPAIR:
        TIDL.int32_serialize(out, 32);
        break;
      case BEC_DH_EXCHANGE:
        TIDL.int32_serialize(out, 48);
        break;
      case BEC_GCM_INIT:
        TIDL.int32_serialize(out, 80);
        break;
      case BEC_GCM_DOBLOCK:
        TIDL.int32_serialize(out, 96);
        break;
    }
  }
  
  public static BulkEncryptionCommand Deserialize(ByteBuffer in) throws TIDL.TIDLException
  {
    BulkEncryptionCommand ret;
    int v = TIDL.int32_deserialize(in);
    
    switch (v)
    {
      case 16:
        ret = BulkEncryptionCommand.BEC_LOGOUT;
        break;
      case 32:
        ret = BulkEncryptionCommand.BEC_GEN_DHPAIR;
        break;
      case 48:
        ret = BulkEncryptionCommand.BEC_DH_EXCHANGE;
        break;
      case 80:
        ret = BulkEncryptionCommand.BEC_GCM_INIT;
        break;
      case 96:
        ret = BulkEncryptionCommand.BEC_GCM_DOBLOCK;
        break;
      default: throw new TIDL.UnrecognizedEnumException(v);
    }
    
    return ret;
  }
}
