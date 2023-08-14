using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using EpicGames.Core;
using UnrealBuildTool;

public class ZenoRemote : ModuleRules
{
    public ZenoRemote(ReadOnlyTargetRules Target) : base(Target)
    {
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore"
            }
        );
        
        AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL");
        AddEngineThirdPartyPrivateStaticDependencies(Target, "zlib");
        
        PrivateIncludePaths.AddRange(new string[]
        {
            Path.Combine(ModuleDirectory, "Private/Generated"),
            Path.Combine(ThirdPartyRoot, "protobuf/include"),
            Path.Combine(ThirdPartyRoot, "grpc/include")
        });
        
        PublicAdditionalLibraries.AddRange(new string[]
        {
            Path.Combine(GRpcStaticLinkFolder, "grpc.lib"),
            Path.Combine(GRpcStaticLinkFolder, "grpc++.lib"),
            Path.Combine(GRpcStaticLinkFolder, "gpr.lib"),
            Path.Combine(GRpcStaticLinkFolder, "address_sorting.lib"),
            Path.Combine(ProtobufStaticLinkFolder, "libprotobuf.lib"),
            Path.Combine(ProtobufStaticLinkFolder, "libprotobuf-lite.lib"),
            Path.Combine(ProtobufStaticLinkFolder, "libprotoc.lib"),
        });
        
        RuntimeDependencies.Add(Path.Combine(BinaryFolder, "libprotobuf.dll"), Path.Combine(ProtobufDynamicLinkFolder, "libprotobuf.dll"));
        // RuntimeDependencies.Add("libprotobuf-lite.dll", Path.Combine(ProtobufDynamicLinkFolder, "libprotobuf-lite.dll"));
        // RuntimeDependencies.Add("libprotoc.dll", Path.Combine(ProtobufDynamicLinkFolder, "libprotoc.dll"));

        PrivateDefinitions.Add("GOOGLE_PROTOBUF_INTERNAL_DONATE_STEAL_INLINE=0");
        
        GenerateProtoFiles();
    }

    protected string PluginPath => Path.GetFullPath(Path.GetDirectoryName(Path.Combine(ModuleDirectory, "../../"))!);

    protected string ThirdPartyRoot => Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty"));
    
    protected string GRpcResourcePath => Path.GetFullPath(Path.Combine(PluginPath, "Resources/gRPC"));
    
    protected string ProtoCExePath => Path.GetFullPath(Path.Combine(GRpcResourcePath, "protoc.exe"));
    
    protected string ProtoCppExePath => Path.GetFullPath(Path.Combine(GRpcResourcePath, "grpc_cpp_plugin.exe"));
    
    protected string ProtoTurboLinkExePath => Path.GetFullPath(Path.Combine(GRpcResourcePath, "protoc-gen-turbolink.exe"));

    protected string DismissHeaderFile => File.ReadAllText(Path.GetFullPath(Path.Combine(GRpcResourcePath, "DismissWarning.h")));
    
    protected string DismissCppFile => File.ReadAllText(Path.GetFullPath(Path.Combine(GRpcResourcePath, "DismissWarning.cpp")));
    
    protected string GenerateCppTargetFolder => Path.GetFullPath(Path.Combine(ModuleDirectory, "Private/Generated"));
    
    protected string ProtoFilesPath => Path.GetFullPath(Path.Combine(GRpcResourcePath, "proto"));
    
    protected string ExternalProtoFilesFolder => Path.GetFullPath(Path.Combine(ThirdPartyRoot, "protobuf/include"));
    
    protected string ProtobufStaticLinkFolder => Path.GetFullPath(Path.Combine(ThirdPartyRoot, "protobuf/lib"));
    
    protected string ProtobufDynamicLinkFolder => Path.GetFullPath(Path.Combine(ThirdPartyRoot, "protobuf/bin"));
    
    protected string GRpcStaticLinkFolder => Path.GetFullPath(Path.Combine(ThirdPartyRoot, "grpc/lib"));

    protected string BinaryFolder => Path.GetFullPath(Path.Combine(PluginPath, "Binaries/Win64"));
    
    protected void GenerateProtoFiles()
    {
        if (!File.Exists(ProtoCExePath) || !File.Exists(ProtoCppExePath))
        {
            Log.TraceError("Protobuf ('{0}') and grpc ('{1}') generator is not found", ProtoCExePath, ProtoCppExePath);
            return;
        }
        
        Log.TraceInformation("Generating gRPC files...");

        var protoFileDir = new DirectoryInfo(ProtoFilesPath);
        var files = protoFileDir.GetFiles("*.proto", SearchOption.AllDirectories);

        Directory.CreateDirectory(ExternalProtoFilesFolder);

        foreach (var fileInfo in files)
        {
            using var pProcess = new Process();
            pProcess.StartInfo.FileName = ProtoCExePath;
            pProcess.StartInfo.ArgumentList.Add($"--proto_path={ExternalProtoFilesFolder}");
            pProcess.StartInfo.ArgumentList.Add($"--proto_path={ProtoFilesPath}");
            pProcess.StartInfo.ArgumentList.Add($"--cpp_out={GenerateCppTargetFolder}");
            pProcess.StartInfo.ArgumentList.Add($"--plugin={ProtoCppExePath} --grpc_out={GenerateCppTargetFolder}");
            // pProcess.StartInfo.ArgumentList.Add($"--plugin={ProtoTurboLinkExePath} --grpc_out={GenerateCppTargetFolder}");
            pProcess.StartInfo.ArgumentList.Add($"{fileInfo.FullName}");
            pProcess.StartInfo.UseShellExecute = false;
            pProcess.StartInfo.RedirectStandardOutput = true;
            pProcess.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
            pProcess.StartInfo.CreateNoWindow = true;
            pProcess.Start();
            var output = pProcess.StandardOutput.ReadToEnd();
            Log.TraceInformation($"Compiling '{fileInfo.Name}':\n{output}");
        }
        
        AddDismissTextToFiles();
    }

    protected void AddDismissTextToFiles()
    {
        Log.TraceInformation("Modifying output to dismiss warning...");
        
        var generatedCppDir = new DirectoryInfo(GenerateCppTargetFolder);
        var headerFiles = generatedCppDir.GetFiles("*.pb.h", SearchOption.AllDirectories);
        var cppFiles = generatedCppDir.GetFiles("*.pb.cc", SearchOption.AllDirectories);

        foreach (var file in headerFiles)
        {
            var currentContent = File.ReadAllText(file.FullName);
            File.WriteAllText(file.FullName, DismissHeaderFile + currentContent);
        }
        
        foreach (var file in cppFiles)
        {
            var currentContent = File.ReadAllText(file.FullName);
            File.WriteAllText(file.FullName, DismissCppFile + currentContent);
        }
    }
}