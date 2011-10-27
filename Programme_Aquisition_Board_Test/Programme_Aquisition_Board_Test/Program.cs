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

                //acqBoardConfiguration.Set_Histogram_8bits(8192, 0, 1500.0, 0, false, false);

                //acqBoardConfiguration.Set_Correlation_8bits(8192, 0, 1500.0, false, false, 3);
                //acqBoardConfiguration.Set_tau_array(0, 0);
                //acqBoardConfiguration.Set_tau_array(1, -10);
                //acqBoardConfiguration.Set_tau_array(2, 10);

                //acqBoardConfiguration.Set_Network_Analyser(true, 0, 1500.0, false, false, 1000000.0);

                //acqBoardConfiguration.Set_Oscilloscope(true, 0, 1500.0, false, false, true, 0, 0.1, true, 1024, 0, 1);
                
                acqBoard.Set_Configuration(acqBoardConfiguration);
                acqBoard.Start_Acq_Module();
                


                //Console.WriteLine(value);

                string wait = Console.ReadLine();
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
