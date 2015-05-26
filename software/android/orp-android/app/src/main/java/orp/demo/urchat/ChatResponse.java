/**
 * @file ChatResponse.java
 */

package orp.demo.urchat;

import orp.orp.TIDL;
import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

public enum ChatResponse
{
  CR_ERROR (255),
  CR_PUBKEY (0),
  CR_GOODBYE (17),
  CR_SEEKUPDATED (33),
  CR_SEEK (49),
  CR_CHANNEL (65),
  CR_CHANNEL_CLOSED (81),
  CR_EXCHANGE (97),
  CR_CIPHERTEXT (113),
  CR_FOUND (129),
  CR_EXCHANGED (130),
  CR_PLAINTEXT (131),
  CR_NEVERMIND (132);
  private final int value;
  
  private ChatResponse(int in_value)
  {
    value = in_value;
  }
  
  public int getValue()
  {
    return value;
  }
  
  public static void Serialize(ByteArrayOutputStream out, ChatResponse in) throws TIDL.TIDLException
  {
    switch (in)
    {
      case CR_ERROR:
        TIDL.int32_serialize(out, 255);
        break;
      case CR_PUBKEY:
        TIDL.int32_serialize(out, 0);
        break;
      case CR_GOODBYE:
        TIDL.int32_serialize(out, 17);
        break;
      case CR_SEEKUPDATED:
        TIDL.int32_serialize(out, 33);
        break;
      case CR_SEEK:
        TIDL.int32_serialize(out, 49);
        break;
      case CR_CHANNEL:
        TIDL.int32_serialize(out, 65);
        break;
      case CR_CHANNEL_CLOSED:
        TIDL.int32_serialize(out, 81);
        break;
      case CR_EXCHANGE:
        TIDL.int32_serialize(out, 97);
        break;
      case CR_CIPHERTEXT:
        TIDL.int32_serialize(out, 113);
        break;
      case CR_FOUND:
        TIDL.int32_serialize(out, 129);
        break;
      case CR_EXCHANGED:
        TIDL.int32_serialize(out, 130);
        break;
      case CR_PLAINTEXT:
        TIDL.int32_serialize(out, 131);
        break;
      case CR_NEVERMIND:
        TIDL.int32_serialize(out, 132);
        break;
    }
  }
  
  public static ChatResponse Deserialize(ByteBuffer in) throws TIDL.TIDLException
  {
    ChatResponse ret;
    int v = TIDL.int32_deserialize(in);
    
    switch (v)
    {
      case 255:
        ret = ChatResponse.CR_ERROR;
        break;
      case 0:
        ret = ChatResponse.CR_PUBKEY;
        break;
      case 17:
        ret = ChatResponse.CR_GOODBYE;
        break;
      case 33:
        ret = ChatResponse.CR_SEEKUPDATED;
        break;
      case 49:
        ret = ChatResponse.CR_SEEK;
        break;
      case 65:
        ret = ChatResponse.CR_CHANNEL;
        break;
      case 81:
        ret = ChatResponse.CR_CHANNEL_CLOSED;
        break;
      case 97:
        ret = ChatResponse.CR_EXCHANGE;
        break;
      case 113:
        ret = ChatResponse.CR_CIPHERTEXT;
        break;
      case 129:
        ret = ChatResponse.CR_FOUND;
        break;
      case 130:
        ret = ChatResponse.CR_EXCHANGED;
        break;
      case 131:
        ret = ChatResponse.CR_PLAINTEXT;
        break;
      case 132:
        ret = ChatResponse.CR_NEVERMIND;
        break;
      default: throw new TIDL.UnrecognizedEnumException(v);
    }
    
    return ret;
  }
}
