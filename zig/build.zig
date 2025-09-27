const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const lib = b.addStaticLibrary(.{ .name = "my_zig_lib", .root_source_file = b.path("src/lib.zig"), .target = target, .optimize = optimize });
    lib.linkLibCpp();
    b.installArtifact(lib);

    const install_header = b.addInstallFile(
        b.path("include/my_zig_lib.h"),
        "include/my_zig_lib.h",
    );
    b.getInstallStep().dependOn(&install_header.step);
}
