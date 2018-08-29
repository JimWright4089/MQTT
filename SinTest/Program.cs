using System;
using System.Threading;
using System.Text;
using System.Runtime.InteropServices;

namespace SinTest
{
  class Program
  {
    public const string DLLLib = "MQTTInterface.dll";
    public const int MAX_ARRAY_LENGTH = 1024;

    [DllImport(DLLLib, CallingConvention = CallingConvention.Cdecl, EntryPoint = "OpenServer")]
    public static extern UInt32 OpenServer(String clientName, String ip, UInt16 port, UInt16 keepAlive);

    [DllImport(DLLLib, CallingConvention = CallingConvention.Cdecl, EntryPoint = "Publish")]
    public static extern UInt32 Publish(string topic,
                                        byte[] message,
                                        UInt16 messageLength,
                                        byte qos,
                                        byte retain);

    static void Main(string[] args)
    {
      Console.WriteLine(OpenServer("SinTest", "jimsserver02.local", 1883, 60000));
      double count = 0.0;

      while (true)
      {
        double sin = Math.Sin(count);
        Int16 int16Sin = (Int16)(sin * (double)1024);
        Int32 int32Sin = (Int32)(sin * (double)1024);
        count += 0.01;
        byte[] theData1 = Message.GetBytes(int16Sin);
        Publish("test/i16sin", theData1, (UInt16)theData1.Length, 0, 1);

        byte[] theData2 = Message.GetBytes(int32Sin);
        Publish("test/i32sin", theData2, (UInt16)theData2.Length, 0, 1);

        byte[] theData3 = Encoding.UTF8.GetBytes(count.ToString());
        Publish("test/sintext", theData3, (UInt16)theData3.Length, 0, 1);

        Thread.Sleep(10);
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
}
