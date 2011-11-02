using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Programme_Aquisition_Board_Test
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                Acquisition_Board_Dll.Acq_Board_Program acqBoard = new Acquisition_Board_Dll.Acq_Board_Program();
                Acquisition_Board_Dll.Acquistion_Configuration acqBoardConfiguration = new Acquisition_Board_Dll.Acquistion_Configuration();


                //bool config_ok = acqBoardConfiguration.Set_Histogram_8bits(1, 0, 1500.0, 1, false, true);

                bool config_ok = acqBoardConfiguration.Set_Correlation_8bits(1, 0, 1500.0, false, false, 3, true);
                acqBoardConfiguration.Set_tau_array(0, 0);
                acqBoardConfiguration.Set_tau_array(1, -2);
                acqBoardConfiguration.Set_tau_array(2, 2);

                //bool config_ok = acqBoardConfiguration.Set_Network_Analyser(true, 0, 1500.0, false, false, 20000.0);

                //acqBoardConfiguration.Set_Oscilloscope(true, 0, 1500.0, false, false, true, 0, 0.1, true, 1024, 0, 1);

                if (config_ok)
                {
                    int error = acqBoard.Set_Configuration(acqBoardConfiguration);
                    if (error == 0)
                    {
                        acqBoard.Start_Acq_Module();
                    }
                }

                //Console.WriteLine(value);

                //string wait = Console.ReadLine();
            }
            catch (Exception ex)
            {
                string message = ex.Message;
                Console.WriteLine(ex.Message);
                Console.ReadLine();
            }
        }
    }
}
