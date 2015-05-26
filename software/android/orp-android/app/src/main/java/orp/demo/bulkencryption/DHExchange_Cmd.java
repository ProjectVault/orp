/**
 * @file DHExchange_Cmd.java
 */

package orp.demo.bulkencryption;

import orp.orp.TIDL;
import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

public class DHExchange_Cmd
{
  public static final int REMOTE_PUBKEY_LENGTH = 128;
  
  public byte[] remote_pubkey;
  public byte local_privkey_idx;
  
  public static void Serialize(ByteArrayOutputStream out, DHExchange_Cmd in, int recdepth) throws TIDL.TIDLException
  {
    if (0 == recdepth) throw new TIDL.RecursionException();
    
    if (in.remote_pubkey.length != REMOTE_PUBKEY_LENGTH) throw new TIDL.ArrayLenException(in.remote_pubkey.length);
    for (int i = 0; i < REMOTE_PUBKEY_LENGTH; i++)
      TIDL.uint8_serialize(out, in.remote_pubkey[i]);
    TIDL.uint8_serialize(out, in.local_privkey_idx);
  }
  
  public static DHExchange_Cmd Deserialize(ByteBuffer in, int recdepth) throws TIDL.TIDLException
  {
    if (0 == recdepth) throw new TIDL.RecursionException();
    
    DHExchange_Cmd ret = new DHExchange_Cmd();
    
    ret.remote_pubkey = new byte[REMOTE_PUBKEY_LENGTH];
    for (int i = 0; i < REMOTE_PUBKEY_LENGTH; i++)
      ret.remote_pubkey[i] = TIDL.uint8_deserialize(in);
    ret.local_privkey_idx = TIDL.uint8_deserialize(in);
    
    return ret;
  }
}
