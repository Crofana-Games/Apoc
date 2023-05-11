using System.Collections.Generic;
using System.IO;
using System.Linq;
using UnrealBuildTool;

public class DotNet : ModuleRules
{
	public DotNet(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "inc"));

		var DotnetDir = Path.Combine("$(BinaryOutputDir)", "dotnet");
		var HostfxrDir = Path.Combine("host", "fxr", "7.0.5", "hostfxr.dll");
		RuntimeDependencies.Add(Path.Combine(DotnetDir, HostfxrDir),
			Path.Combine(ModuleDirectory, HostfxrDir));

		var Files = GetFiles(Path.Combine(ModuleDirectory, "shared", "Microsoft.NETCore.App", "7.0.5"));
		foreach (var File in Files)
		{
			var DestPath = File.Substring(ModuleDirectory.Length + 1,
				File.Length - ModuleDirectory.Length - 1);

			RuntimeDependencies.Add(Path.Combine(DotnetDir, DestPath), File);
		}
	}
	
	private static IEnumerable<string> GetFiles(string InDirectory, string InPattern = "*.*")
	{
		var Files = new List<string>();

		var Strings = Directory.GetFiles(InDirectory, InPattern);

		foreach (var File in Strings)
		{
			Files.Add(File);
		}

		foreach (var File in Directory.GetDirectories(InDirectory))
		{
			Files.AddRange(GetFiles(File, InPattern));
		}

		return Files;
	}
}