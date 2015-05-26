/**
 * @file GenDHPair_Res.java
 */

package orp.demo.bulkencryption;

import orp.orp.TIDL;
import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

public class GenDHPair_Res
{
  public static final int PUBKEY_LENGTH = 128;
  
  public byte[] pubkey;
  public byte privkey_idx;
  
  public static void Serialize(ByteArrayOutputStream out, GenDHPair_Res in, int recdepth) throws TIDL.TIDLException
  {
    if (0 == recdepth) throw new TIDL.RecursionException();
    
    if (in.pubkey.length != PUBKEY_LENGTH) throw new TIDL.ArrayLenException(in.pubkey.length);
    for (int i = 0; i < PUBKEY_LENGTH; i++)
      TIDL.uint8_serialize(out, in.pubkey[i]);
    TIDL.uint8_serialize(out, in.privkey_idx);
  }
  
  public static GenDHPair_Res Deserialize(ByteBuffer in, int recdepth) throws TIDL.TIDLException
  {
    if (0 == recdepth) throw new TIDL.RecursionException();
    
    GenDHPair_Res ret = new GenDHPair_Res();
    
    ret.pubkey = new byte[PUBKEY_LENGTH];
    for (int i = 0; i < PUBKEY_LENGTH; i++)
      ret.pubkey[i] = TIDL.uint8_deserialize(in);
    ret.privkey_idx = TIDL.uint8_deserialize(in);
    
    return ret;
  }
}
