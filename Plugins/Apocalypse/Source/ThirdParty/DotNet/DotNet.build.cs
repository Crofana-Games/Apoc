using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;
using UnrealBuildTool;

public class DotNet : ModuleRules
{
	public DotNet(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		
		var DotnetDir = Path.Combine("$(BinaryOutputDir)", "dotnet");

		var Files = GetFiles(ModuleDirectory);
		foreach (var File in Files)
		{
			if (Regex.IsMatch(File, ".*\\.build\\.cs"))
			{
				continue;
			}
			
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