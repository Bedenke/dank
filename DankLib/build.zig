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
    const target = b.standardTargetOptions(.{});

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

    const libWindowsStatic = b.addStaticLibrary(.{
        .name = "DankWindows",
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    libWindowsStatic.addIncludePath(b.path("src/"));
    libWindowsStatic.linkLibCpp();
    libWindowsStatic.linkSystemLibrary("d3d12");
    libWindowsStatic.linkSystemLibrary("dxgi");

    libWindowsStatic.addCSourceFiles(.{
        .root = b.path("src"),
        .files = &.{
            "modules/engine/Console.cpp",
            "modules/engine/Engine.cpp",
            "modules/scene/Scene.cpp",
            "modules/scene/Camera.cpp",
            "modules/input/Input.cpp",
            "os/windows/WindowsOS.cpp",
            "os/windows/DankView.cpp",
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
    exe.addIncludePath(b.path("src/"));
    exe.linkLibrary(libWindowsStatic);
    exe.linkLibC();
    exe.linkLibCpp();

    targets.append(libWindowsStatic) catch |err| {
        std.debug.print("Failed {s}", .{@errorName(err)});
    };

    targets.append(exe) catch |err| {
        std.debug.print("Failed {s}", .{@errorName(err)});
    };

    b.installArtifact(libWindowsStatic);
    b.installArtifact(exe);

    zcc.createStep(b, "cdb", targets.toOwnedSlice() catch @panic("OOM"));
}
