const std = @import("std");

const Allocator = std.mem.Allocator;

export fn zig_add(a: i32, b: i32) i32 {
    return a + b;
}

pub const GameState = struct {
    const Self = @This();
    allocator: *Allocator,
    cppPtr: usize,

    pub fn createGameObject(self: *Self, objectName: []const u8, objectType: []const u8) void {
        const nullTerminatedName = std.cstr.addNullByte(self.allocator, objectName) catch unreachable;
        defer self.allocator.free(nullTerminatedName);

        const nullTerminatedType = std.cstr.addNulByte(self.allocator, objectType) catch unreachable;
        defer self.allocator.free(nullTerminatedType);

        //const propagate = true;

    }
};

pub const ZigPoint = extern struct {
    x: f32,
    y: f32,

    pub export fn create(x: f32, y: f32) ZigPoint {
        return ZigPoint{ .x = x, .y = y };
    }

    pub export fn magnitude(self: *const ZigPoint) f32 {
        return @sqrt(self.x * self.x + self.y * self.y);
    }
};
