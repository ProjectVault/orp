/**
 * @file DHExchange_Res.java
 */

package orp.demo.bulkencryption;

import orp.orp.TIDL;
import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

public class DHExchange_Res
{
  ;
  
  public byte shared_idx;
  
  public static void Serialize(ByteArrayOutputStream out, DHExchange_Res in, int recdepth) throws TIDL.TIDLException
  {
    if (0 == recdepth) throw new TIDL.RecursionException();
    
    TIDL.uint8_serialize(out, in.shared_idx);
  }
  
  public static DHExchange_Res Deserialize(ByteBuffer in, int recdepth) throws TIDL.TIDLException
  {
    if (0 == recdepth) throw new TIDL.RecursionException();
    
    DHExchange_Res ret = new DHExchange_Res();
    
    ret.shared_idx = TIDL.uint8_deserialize(in);
    
    return ret;
  }
}
