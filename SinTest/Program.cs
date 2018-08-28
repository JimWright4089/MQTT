using System;
using System.Text;
using System.Threading;
using MQTTCSInterface;

namespace SinTest
{
  class Program
  {
    static void Main(string[] args)
    {
      Console.WriteLine(MQTTInterface.OpenServer("SinTest", "jimsserver02.local", 1883, 60000));
      double count = 0.0;

      while (true)
      {
        double sin = Math.Sin(count);
        Int16 int16Sin = (Int16)(sin * (double)1024);
        Int32 int32Sin = (Int32)(sin * (double)1024);
        count += 0.01;
        byte[] theData1 = Message.GetBytes(int16Sin);
        MQTTInterface.Publish("test/i16sin", theData1, (UInt16)theData1.Length, 0, 1);

        byte[] theData2 = Message.GetBytes(int32Sin);
        MQTTInterface.Publish("test/i32sin", theData2, (UInt16)theData2.Length, 0, 1);

        byte[] theData3 = Encoding.UTF8.GetBytes(sin.ToString() + "\n");
        MQTTInterface.Publish("test/sintext", theData3, (UInt16)theData3.Length, 0, 1);

        Thread.Sleep(10);
      }
    }
  }
}
