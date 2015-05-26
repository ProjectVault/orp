/**
 * @file GenDHPair_Cmd.java
 */

package orp.demo.bulkencryption;

import orp.orp.TIDL;
import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

public class GenDHPair_Cmd
{
  public static final int SEED_LENGTH = 32;
  
  public byte[] seed;
  
  public static void Serialize(ByteArrayOutputStream out, GenDHPair_Cmd in, int recdepth) throws TIDL.TIDLException
  {
    if (0 == recdepth) throw new TIDL.RecursionException();
    
    if (in.seed.length != SEED_LENGTH) throw new TIDL.ArrayLenException(in.seed.length);
    for (int i = 0; i < SEED_LENGTH; i++)
      TIDL.uint8_serialize(out, in.seed[i]);
  }
  
  public static GenDHPair_Cmd Deserialize(ByteBuffer in, int recdepth) throws TIDL.TIDLException
  {
    if (0 == recdepth) throw new TIDL.RecursionException();
    
    GenDHPair_Cmd ret = new GenDHPair_Cmd();
    
    ret.seed = new byte[SEED_LENGTH];
    for (int i = 0; i < SEED_LENGTH; i++)
      ret.seed[i] = TIDL.uint8_deserialize(in);
    
    return ret;
  }
}
