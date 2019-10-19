using NesEmu.Emulator;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace NesEmu
{
    static class Program
    {
        /// <summary>
        ///  The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            var system = new EntertainmentSystem();

            if (args.Length != 0)
            {
                using (var stream = File.OpenRead(args[0]))
                {
                    var cart = Cart.Load(stream);
                    system.InsertCart(cart);
                }
            }

            system.Reset();

            Application.SetHighDpiMode(HighDpiMode.SystemAware);
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new EmulatorForm() { EntertainmentSystem = system });
        }
    }
}
