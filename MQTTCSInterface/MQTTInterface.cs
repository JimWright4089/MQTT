using System;
using System.Runtime.InteropServices;

namespace MQTTCSInterface
{
  public class MQTTInterface
  {
    public const string DLLLib = ".\\MQTTInterfaceDLL.dll";
    public const int MAX_ARRAY_LENGTH = 1024;

    [DllImport(DLLLib, CallingConvention = CallingConvention.Cdecl, EntryPoint = "InitServer")]
    public static extern UInt32 InitServer(String clientName);
    [DllImport(DLLLib, CallingConvention = CallingConvention.Cdecl, EntryPoint = "OpenServer")]
    public static extern UInt32 OpenServer(String ip, UInt16 port, UInt16 keepAlive);
    [DllImport(DLLLib, CallingConvention = CallingConvention.Cdecl, EntryPoint = "CloseServer")]
    public static extern UInt32 CloseServer();
    [DllImport(DLLLib, CallingConvention = CallingConvention.Cdecl, EntryPoint = "ReceiveLoop")]
    public static extern UInt32 ReceiveLoop();

    [DllImport(DLLLib, CallingConvention = CallingConvention.Cdecl, EntryPoint = "Publish")]
    public static extern UInt32 Publish(string topic,
                                        byte[] message,
                                        UInt16 messageLength,
                                        byte qos,
                                        byte retain);
    [DllImport(DLLLib, CallingConvention = CallingConvention.Cdecl, EntryPoint = "Subscribe")]
    public static extern UInt32 Subscribe(string topic, byte qos);
    [DllImport(DLLLib, CallingConvention = CallingConvention.Cdecl, EntryPoint = "UnSubscribe")]
    public static extern UInt32 UnSubscribe(string topic);
    [DllImport(DLLLib, CallingConvention = CallingConvention.Cdecl, EntryPoint = "ReceiveMessage")]
    public static extern UInt32 ReceiveRawMessage(ref UInt16 messageID,
                                                byte[] message,
                                                ref UInt16 messageLength,
                                                byte[] topic,
                                                ref UInt16 topicLength,
                                                ref byte qos,
                                                ref byte retain);

    public static UInt32 ReceiveMessage(ref Message message)
    {
      UInt16 messageID = 0;
      UInt16 topicLength = MAX_ARRAY_LENGTH;
      byte[] topic = new byte[topicLength];
      UInt16 messageLength = MAX_ARRAY_LENGTH;
      byte[] messageBytes = new byte[messageLength];
      byte qos = 0;
      byte retain = 0;

      if (0 == ReceiveRawMessage(ref messageID,
                                messageBytes,
                                ref messageLength,
                                topic,
                                ref topicLength,
                                ref qos,
                                ref retain))
      {
        message.mID = messageID;
        message.mPacketLength = messageLength;
        message.mQulityOfService = qos;
        message.mRetain = (retain == 0) ? false : true;
        message.mPacket = new byte[message.mPacketLength];
        for (UInt16 i = 0; i < message.mPacketLength; i++)
        {
          message.mPacket[i] = messageBytes[i];
        }
        message.mTopic = "";
        message.mTopic = System.Text.Encoding.ASCII.GetString(topic);
        message.mTopic = message.mTopic.Substring(0, topicLength);
      }
      else
      {
        return 1;
      }

      return 0;
    }

  }

  public class Message
  {
    public string mTopic = "";
    public UInt16 mPacketLength = 0;
    public byte[] mPacket;
    public UInt16 mID = 0;
    public bool mRetain = false;
    public UInt16 mQulityOfService = 0;

    public double GetDouble()
    {
      double returnValue = 0;
      try
      {
        returnValue = BitConverter.ToDouble(mPacket, 0);
      }
      catch (Exception)
      {

      }
      return returnValue;
    }
    public double GetInt16()
    {
      if (null != mPacket)
      {
        return BitConverter.ToInt16(mPacket, 0);
      }
      return 0.0;
    }
    public double GetInt32()
    {
      return BitConverter.ToInt32(mPacket, 0);
    }

    public static byte[] GetBytes(Int16 data)
    {
      return BitConverter.GetBytes(data);
    }

    public static byte[] GetBytes(Int32 data)
    {
      return BitConverter.GetBytes(data);
    }

    public static byte[] GetBytes(double data)
    {
      return BitConverter.GetBytes(data);
    }
  }
}
