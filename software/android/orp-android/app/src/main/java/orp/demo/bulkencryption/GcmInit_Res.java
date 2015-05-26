/**
 * @file GcmInit_Res.java
 */

package orp.demo.bulkencryption;

import orp.orp.TIDL;
import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

public class GcmInit_Res
{
  ;
  
  public byte gcm_idx;
  
  public static void Serialize(ByteArrayOutputStream out, GcmInit_Res in, int recdepth) throws TIDL.TIDLException
  {
    if (0 == recdepth) throw new TIDL.RecursionException();
    
    TIDL.uint8_serialize(out, in.gcm_idx);
  }
  
  public static GcmInit_Res Deserialize(ByteBuffer in, int recdepth) throws TIDL.TIDLException
  {
    if (0 == recdepth) throw new TIDL.RecursionException();
    
    GcmInit_Res ret = new GcmInit_Res();
    
    ret.gcm_idx = TIDL.uint8_deserialize(in);
    
    return ret;
  }
}
