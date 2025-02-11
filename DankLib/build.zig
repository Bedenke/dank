const std = @import("std");
const zcc = @import("compile_commands");

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = blk: {
        var tgt = b.standardTargetOptions(.{});
        if (tgt.result.os.tag == .windows) {
            tgt.result.abi = .msvc; // Force MSVC ABI only on Windows
        }
        break :blk tgt;
    };
    // Standard optimization options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall. Here we do not
    // set a preferred release mode, allowing the user to decide how to optimize.
    const optimize = b.standardOptimizeOption(.{});

    buildWindows(b, target, optimize);
}

pub fn buildApple(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode) void {
    var targets = std.ArrayList(*std.Build.Step.Compile).init(b.allocator);

    const time_in_sec = std.time.timestamp();
    const fmt = "{s}_{}";
    var buf: [100]u8 = undefined;
    const name = std.fmt.bufPrint(&buf, fmt, .{ "dank", time_in_sec }) catch "engine";

    const cflags = [_][]const u8{ "-std=c++17", "-fno-sanitize=undefined" };

    const libAppleDynamic = b.addSharedLibrary(.{
        .name = name,
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    libAppleDynamic.addIncludePath(b.path("src/"));
    libAppleDynamic.installHeader(b.path("src/os/apple/DankView.h"), "DankView.h");
    libAppleDynamic.linkLibCpp();
    libAppleDynamic.linkFramework("Foundation");
    libAppleDynamic.linkFramework("Metal");

    libAppleDynamic.addCSourceFiles(.{
        .root = b.path("src"),
        .files = &.{
            "modules/engine/Console.cpp",
            "modules/engine/Engine.cpp",
            "modules/scene/Scene.cpp",
            "modules/scene/Camera.cpp",
            "modules/os/Thread.cpp",
            "modules/input/Input.cpp",
            "os/apple/AppleOS.cpp",
            "os/apple/renderer/AppleRenderer.cpp",
        },
        .flags = &cflags,
    });

    const objCFlags = [_][]const u8{ "-std=c++17", "-fno-sanitize=undefined", "-fobjc-arc" };

    const libAppleStatic = b.addStaticLibrary(.{
        .name = "DankApple",
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    libAppleStatic.addIncludePath(b.path("src/"));
    libAppleStatic.addCSourceFiles(.{
        .root = b.path("src"),
        .files = &.{
            "os/apple/support/CaptureStreamOutput.mm",
            "os/apple/support/CaptureEngine.mm",
            "os/apple/DankView.mm",
        },
        .flags = &objCFlags,
    });
    libAppleStatic.linkLibCpp();
    libAppleStatic.linkFramework("Foundation");
    libAppleStatic.linkFramework("CoreFoundation");
    libAppleStatic.linkFramework("Metal");
    libAppleStatic.linkFramework("MetalKit");
    libAppleStatic.linkFramework("ScreenCaptureKit");

    const HelperFunctions = struct {
        fn clearLibDir(_: *std.Build.Step, _: std.Progress.Node) anyerror!void {
            const cwd = std.fs.cwd();
            try cwd.deleteTree("zig-out/lib/");
            try cwd.makeDir("zig-out/lib/");
        }
    };

    var clean_lib_step = std.Build.Step.init(.{
        .id = .custom,
        .name = "clean-lib-directory",
        .owner = b,
        .makeFn = HelperFunctions.clearLibDir,
    });

    const tmp = b.makeTempPath();

    //xcrun -sdk macosx metal -o Shadow.ir  -c Shadow.metal
    const compiled_shader = b.fmt("{s}/{s}", .{ tmp, "StaticMesh.ir" });
    const compile_shader = b.addSystemCommand(&.{"xcrun"});
    compile_shader.addArgs(&.{
        "-sdk",
        "macosx",
        "metal",
        "-frecord-sources",
        "-gline-tables-only",
        "-o",
        compiled_shader,
        "-c",
    });
    compile_shader.addFileArg(b.path("src/os/apple/renderer/shaders/StaticMesh.metal"));
    compile_shader.expectExitCode(0);

    const pack_name = std.fmt.bufPrint(&buf, "{s}_{}.{s}", .{ "Dank", time_in_sec, "metallib" }) catch "metal";

    const pack_shaders = b.addSystemCommand(&.{"xcrun"});
    pack_shaders.addArgs(&.{
        "-sdk",
        "macosx",
        "metallib",
        "-o",
    });
    const p = b.pathJoin(&.{
        ".",
        "zig-out",
        "lib",
        pack_name,
    });
    // const p = b.pathJoin(&.{
    //     "..",
    //     "DankApple",
    //     "DankApple Shared",
    //     "Resources",
    //     pack_name,
    // });
    pack_shaders.addArg(p);
    pack_shaders.addArg(b.pathFromRoot(compiled_shader));
    pack_shaders.expectExitCode(0);

    compile_shader.step.dependOn(&clean_lib_step);
    pack_shaders.step.dependOn(&compile_shader.step);

    targets.append(libAppleDynamic) catch |err| {
        std.debug.print("Failed {s}", .{@errorName(err)});
    };

    b.getInstallStep().dependOn(&pack_shaders.step);

    // This declares intent for the library to be installed into the standard
    // location when the user invokes the "install" step (the default step when
    // running `zig build`).
    b.installArtifact(libAppleDynamic);
    b.installArtifact(libAppleStatic);

    zcc.createStep(b, "cdb", targets.toOwnedSlice() catch @panic("OOM"));
}

pub fn buildWindows(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode) void {
    var targets = std.ArrayList(*std.Build.Step.Compile).init(b.allocator);

    const cflags = [_][]const u8{ "-std=c++17", "-fno-sanitize=undefined" };

    const lib = b.addStaticLibrary(.{
        .name = "DankWindows",
        .target = target,
        .optimize = optimize,
    });

    lib.addSystemIncludePath(.{ .cwd_relative = "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.26100.0\\winrt" });
    lib.addIncludePath(b.path("vendor/DirectX-Headers-1.615.0/include/directx/"));
    lib.addIncludePath(b.path("src/"));
    lib.linkSystemLibrary("dxgi");
    lib.linkSystemLibrary("d3d12");
    lib.linkSystemLibrary("d3dcompiler");
    lib.linkSystemLibrary("shlwapi");
    lib.linkSystemLibrary("user32");
    lib.linkLibC();
    // if (b.lazyDependency("directx_headers", .{
    //     .target = target,
    //     .optimize = optimize,
    // })) |dep| {
    //     lib.addIncludePath(b.path("vendor/DirectX-Headers-1.615.0/include/directx/"));
    //     lib.linkLibrary(dep.artifact("directx-headers"));
    // }

    lib.addCSourceFiles(.{
        .root = b.path("src"),
        .files = &.{
            "modules/engine/Engine.cpp",
            "modules/engine/Console.cpp",
            "modules/scene/Scene.cpp",
            "modules/scene/Camera.cpp",
            "modules/input/Input.cpp",
            "os/windows/WindowsOS.cpp",
            "os/windows/DankView.cpp",
            // "os/windows/support/WindowsConsole.cpp",
            "os/windows/support/Thread.cpp",
            "os/windows/renderer/DXRenderer.cpp",
        },
        .flags = &cflags,
    });

    const exe = b.addExecutable(.{
        .name = "DankApp",
        .target = target,
        .optimize = optimize,
    });
    exe.addCSourceFile(.{
        .file = b.path("src/os/windows/main.cpp"),
        .flags = &cflags,
    });

    //exe.addSystemIncludePath(.{ .cwd_relative = "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.26100.0\\winrt" });
    //exe.addSystemIncludePath(.{ .cwd_relative = "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.42.34433\\Lib\x64" });
    //exe.addIncludePath(b.path("vendor/DirectX-Headers-1.615.0/include/directx/"));
    exe.addIncludePath(b.path("src/"));
    exe.linkLibC();
    //exe.linkLibCpp();
    exe.linkSystemLibrary("dxgi");
    exe.linkSystemLibrary("d3d12");
    exe.linkSystemLibrary("d3dcompiler");
    exe.linkSystemLibrary("shlwapi");
    lib.linkSystemLibrary("user32");
    exe.linkLibrary(lib);

    targets.append(lib) catch |err| {
        std.debug.print("Failed {s}", .{@errorName(err)});
    };

    targets.append(exe) catch |err| {
        std.debug.print("Failed {s}", .{@errorName(err)});
    };

    b.installArtifact(lib);
    b.installArtifact(exe);

    const vs_output_file = b.pathJoin(&.{ ".", "zig-out", "bin", "VS_main.cso" });
    const ps_output_file = b.pathJoin(&.{ ".", "zig-out", "bin", "PS_main.cso" });

    const compile_vs_shader = b.addSystemCommand(&.{"dxc"});
    compile_vs_shader.addArgs(&.{ "-T", "vs_5_0", "-E", "VSMain", "-Fo" });
    compile_vs_shader.addArg(vs_output_file);
    compile_vs_shader.addFileArg(b.path("src/os/windows/renderer/shader.hlsl"));
    compile_vs_shader.expectExitCode(0);

    const compile_ps_shader = b.addSystemCommand(&.{"dxc"});
    compile_ps_shader.addArgs(&.{ "-T", "ps_5_0", "-E", "PSMain", "-Fo" });
    compile_ps_shader.addArg(ps_output_file);
    compile_ps_shader.addFileArg(b.path("src/os/windows/renderer/shader.hlsl"));
    compile_ps_shader.expectExitCode(0);

    b.getInstallStep().dependOn(&compile_vs_shader.step);
    b.getInstallStep().dependOn(&compile_ps_shader.step);

    //zcc.createStep(b, "cdb", targets.toOwnedSlice() catch @panic("OOM"));
}
