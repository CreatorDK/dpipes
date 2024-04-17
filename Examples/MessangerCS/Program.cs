namespace MessangerCS
{
    internal static class Program
    {
        [STAThread]
        static void Main()
        {
            string[] args = Environment.GetCommandLineArgs();

            ApplicationConfiguration.Initialize();
            Application.Run(new FormMain(args));
        }
    }
}