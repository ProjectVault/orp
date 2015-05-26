/**
 * @file GcmInit_Cmd.java
 */

package orp.demo.bulkencryption;

import orp.orp.TIDL;
import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

public class GcmInit_Cmd
{
  ;
  
  public GCM_Mode mode;
  public byte shared_idx;
  
  public static void Serialize(ByteArrayOutputStream out, GcmInit_Cmd in, int recdepth) throws TIDL.TIDLException
  {
    if (0 == recdepth) throw new TIDL.RecursionException();
    
    GCM_Mode.Serialize(out, in.mode);
    TIDL.uint8_serialize(out, in.shared_idx);
  }
  
  public static GcmInit_Cmd Deserialize(ByteBuffer in, int recdepth) throws TIDL.TIDLException
  {
    if (0 == recdepth) throw new TIDL.RecursionException();
    
    GcmInit_Cmd ret = new GcmInit_Cmd();
    
    ret.mode = GCM_Mode.Deserialize(in);
    ret.shared_idx = TIDL.uint8_deserialize(in);
    
    return ret;
  }
}
