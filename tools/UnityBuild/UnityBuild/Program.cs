using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace UnityBuild {
    class Program {

        static public String sSrcFullPath;

        static void Main(string[] args) {

            if (args.Length < 1) {
                Console.WriteLine("Usage : .\\unityBuild.exe srcDirectoryName");
                return;
            }

            sSrcFullPath = Path.GetFullPath(args[0]);
            if (sSrcFullPath[sSrcFullPath.Length - 1] != '\\')
                sSrcFullPath = sSrcFullPath + "\\";

            Directory.SetCurrentDirectory(sSrcFullPath);
            DirectoryInfo oDirectory = Directory.CreateDirectory(args[0]);
            String sResult = GetDirectoryIncludes( oDirectory );

            Console.WriteLine( sResult );
            TextWriter tw = new StreamWriter(sSrcFullPath + "unityBuild.cpp");
            tw.WriteLine(sResult);
            tw.Close();
        }

        static public String GetDirectoryIncludes( DirectoryInfo oDirectory ) {
            String sRes = "";

            IEnumerable<FileInfo> oFiles = oDirectory.EnumerateFiles();
            foreach (FileInfo oFile in oFiles )
            {
                if (oFile.Name == "unityBuild.cpp")
                    continue;
                if ((oFile.Extension != ".cpp") && (oFile.Extension != ".c"))
                    continue;
                String sFileRelativePath = oFile.FullName;
                sFileRelativePath = sFileRelativePath.Replace(sSrcFullPath, "");
                if (sFileRelativePath[0] == '\\')
                    sFileRelativePath = sFileRelativePath.Remove(0, 1);
                sRes = sRes + "#include \"" + sFileRelativePath + "\"\n";
            }

            IEnumerable<DirectoryInfo> oDirectories = oDirectory.EnumerateDirectories();
            foreach (DirectoryInfo oSubDirectory in oDirectories) {
                sRes += GetDirectoryIncludes(oSubDirectory);
            }

            return sRes;
        }
    }
}
