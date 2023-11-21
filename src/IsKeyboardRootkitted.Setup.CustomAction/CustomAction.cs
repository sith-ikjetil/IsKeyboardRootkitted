using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Deployment.WindowsInstaller;
using System.Diagnostics;
namespace IsKeyboardRootkitted.Setup.CustomAction
{
    public class CustomActions
    {
        [CustomAction]
        public static ActionResult ExecuteInstallInf(Session session)
        {
            // Get Path To Application
            string customActionData = session["CustomActionData"];

            // Register KKS.KmdIsKeyboardRkt.inf
            ProcessStartInfo startInfoKKRkt = new ProcessStartInfo();
            startInfoKKRkt.WorkingDirectory = customActionData;
            startInfoKKRkt.Verb = "Open";// install";
            startInfoKKRkt.FileName = "devcon";
            startInfoKKRkt.Arguments = @"install ""IsKeyboardRootkitted.Driver.inf"" ""Root\IsKeyboardRootkitted.Driver""";
            using (Process exeProcess = System.Diagnostics.Process.Start(startInfoKKRkt))
            {
                exeProcess.WaitForExit();
            }

            return ActionResult.Success;
        }

        [CustomAction]
        public static ActionResult ExecuteUnInstallInf(Session session)
        {
            // Get Path To Application
            string customActionData = session["CustomActionData"];

            // Register KKS.KmdIsKeyboardRkt.inf
            ProcessStartInfo startInfoKKRkt = new ProcessStartInfo();
            startInfoKKRkt.WorkingDirectory = customActionData;
            startInfoKKRkt.Verb = "Open";// install";
            startInfoKKRkt.FileName = "devcon";
            startInfoKKRkt.Arguments = @"remove ""IsKeyboardRootkitted.Driver.inf"" ""Root\IsKeyboardRootKitted.Driver""";
            using (Process exeProcess = System.Diagnostics.Process.Start(startInfoKKRkt))
            {
                exeProcess.WaitForExit();
            }

            return ActionResult.Success;
        }
    }
}
