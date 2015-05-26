/**
 * @file GcmDoBlock_Res.java
 */

package orp.demo.bulkencryption;

import orp.orp.TIDL;
import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

public class GcmDoBlock_Res
{
  public static final int BLOCK_LENGTH = 16;
  
  public byte[] block;
  
  public static void Serialize(ByteArrayOutputStream out, GcmDoBlock_Res in, int recdepth) throws TIDL.TIDLException
  {
    if (0 == recdepth) throw new TIDL.RecursionException();
    
    if (in.block.length != BLOCK_LENGTH) throw new TIDL.ArrayLenException(in.block.length);
    for (int i = 0; i < BLOCK_LENGTH; i++)
      TIDL.uint8_serialize(out, in.block[i]);
  }
  
  public static GcmDoBlock_Res Deserialize(ByteBuffer in, int recdepth) throws TIDL.TIDLException
  {
    if (0 == recdepth) throw new TIDL.RecursionException();
    
    GcmDoBlock_Res ret = new GcmDoBlock_Res();
    
    ret.block = new byte[BLOCK_LENGTH];
    for (int i = 0; i < BLOCK_LENGTH; i++)
      ret.block[i] = TIDL.uint8_deserialize(in);
    
    return ret;
  }
}
