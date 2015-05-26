/**
 * @file ChatCommand.java
 */

package orp.demo.urchat;

import orp.orp.TIDL;
import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;

public enum ChatCommand
{
  CC_LOGOUT (16),
  CC_SETSEEK (32),
  CC_GETSEEK (48),
  CC_GET_CHANNEL (64),
  CC_CLOSE_CHANNEL (80),
  CC_GET_EXCHANGE (96),
  CC_ENCRYPT (112),
  CC_INCOMING (128);
  private final int value;
  
  private ChatCommand(int in_value)
  {
    value = in_value;
  }
  
  public int getValue()
  {
    return value;
  }
  
  public static void Serialize(ByteArrayOutputStream out, ChatCommand in) throws TIDL.TIDLException
  {
    switch (in)
    {
      case CC_LOGOUT:
        TIDL.int32_serialize(out, 16);
        break;
      case CC_SETSEEK:
        TIDL.int32_serialize(out, 32);
        break;
      case CC_GETSEEK:
        TIDL.int32_serialize(out, 48);
        break;
      case CC_GET_CHANNEL:
        TIDL.int32_serialize(out, 64);
        break;
      case CC_CLOSE_CHANNEL:
        TIDL.int32_serialize(out, 80);
        break;
      case CC_GET_EXCHANGE:
        TIDL.int32_serialize(out, 96);
        break;
      case CC_ENCRYPT:
        TIDL.int32_serialize(out, 112);
        break;
      case CC_INCOMING:
        TIDL.int32_serialize(out, 128);
        break;
    }
  }
  
  public static ChatCommand Deserialize(ByteBuffer in) throws TIDL.TIDLException
  {
    ChatCommand ret;
    int v = TIDL.int32_deserialize(in);
    
    switch (v)
    {
      case 16:
        ret = ChatCommand.CC_LOGOUT;
        break;
      case 32:
        ret = ChatCommand.CC_SETSEEK;
        break;
      case 48:
        ret = ChatCommand.CC_GETSEEK;
        break;
      case 64:
        ret = ChatCommand.CC_GET_CHANNEL;
        break;
      case 80:
        ret = ChatCommand.CC_CLOSE_CHANNEL;
        break;
      case 96:
        ret = ChatCommand.CC_GET_EXCHANGE;
        break;
      case 112:
        ret = ChatCommand.CC_ENCRYPT;
        break;
      case 128:
        ret = ChatCommand.CC_INCOMING;
        break;
      default: throw new TIDL.UnrecognizedEnumException(v);
    }
    
    return ret;
  }
}
