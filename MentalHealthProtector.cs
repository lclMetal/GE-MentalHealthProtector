using System;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace GE_MentalHealthProtector
{
    public class MHPConfig
    {
        public string sourceFolderPath { get; set;}
        public string outputFilePath { get; set; }
    }

    class MentalHealthProtector
    {
        private static MHPConfig config;

        public static void Main()
        {
            const string configFileName = "gemhp-conf.json";

            if (!File.Exists(configFileName))
            {
                using StreamWriter configFile = File.CreateText(configFileName);
                configFile.Write("{\n    \"sourceFolderPath\": \".\\\\source\",\n    \"outputFilePath\": \".\\\\output.c\"\n}");
                configFile.Flush();
                configFile.Close();
            }

            string configStringJSON = File.ReadAllText(configFileName);

            try
            {
                config = JsonSerializer.Deserialize<MHPConfig>(configStringJSON);
            }
            catch (JsonException)
            {
                Console.WriteLine("Badly formed JSON. Make sure that all \"\\\" characters in the file paths are doubled, like so: \\\\");
                Console.WriteLine("Press enter to exit.");
                Console.ReadLine();
                return;
            }
            catch (Exception e)
            {
                Console.WriteLine("Something went wrong. Press enter to exit.");
                Console.ReadLine();
                Console.WriteLine(e.Message);
            }

            if (!Directory.Exists(config.sourceFolderPath))
            {
                Console.WriteLine("Source folder path invalid. Press enter to exit.");
                Console.ReadLine();
                return;
            }

            using FileSystemWatcher watcher = new FileSystemWatcher(config.sourceFolderPath);

            watcher.NotifyFilter = NotifyFilters.FileName
                                 | NotifyFilters.LastWrite;

            watcher.Changed += OnChanged;
            watcher.Created += OnChanged;
            watcher.Deleted += OnChanged;
            watcher.Renamed += OnChanged;
            watcher.IncludeSubdirectories = true;
            watcher.EnableRaisingEvents = true;

            Console.WriteLine("===== [ Game Editor Mental Health Protector ] =====\nMonitoring \"" + config.sourceFolderPath + "\" for changes... Press enter to exit.");

            string command = "";

            do
            {
                command = Console.ReadLine();

                if (command == "update")
                {
                    Update("update requested by user", "");
                }
            }
            while (command != "exit");
        }

        private static void Update(string changeType, string updatedFileName)
        {
            Console.WriteLine("File " + changeType + ": " + updatedFileName + "\nCombining script files...");

            var fileNames = Directory.EnumerateFiles(config.sourceFolderPath).OrderBy(s => s);

            using StreamWriter outputFile = File.CreateText(config.outputFilePath);
            System.Threading.Thread.Sleep(1000);

            foreach (var file in fileNames)
            {
                string fileData = File.ReadAllText(@file);
                outputFile.Write("// " + file + "\n" + fileData + "\n\n");
            }

            Console.WriteLine(fileNames.Count() + " files combined. Result written to: " + config.outputFilePath);
            System.Threading.Thread.Sleep(1000);
            Console.Clear();
            Console.WriteLine("===== [ Game Editor Mental Health Protector ] =====\nMonitoring \"" + config.sourceFolderPath + "\" for changes... Press enter to exit.");
        }

        private static void OnChanged(object sender, FileSystemEventArgs e)
        {
            Update(e.ChangeType.ToString().ToLower(), e.Name);
        }
    }
}
