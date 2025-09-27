using ImGuiNET;

namespace Gameplay
{

    public class SharpClass
    {

        internal nint ContextId { get; private set; }
        public static void Output()
        {

            Console.WriteLine("This is a hello world from c#!");


            ImGui.Begin("Hello, world!");

            ImGui.End();



           
           
        }

        public static void Init(IntPtr context, IntPtr alloc, IntPtr free)
        {

            Console.WriteLine(context.ToString() + " " + alloc.ToString() + " " + free.ToString());
            ImGui.GetCurrentContext();
            ImGui.SetCurrentContext(context);
            ImGui.SetAllocatorFunctions(alloc, free);
        }

        public static unsafe void Initialize()
        {
            return;
        }

    }
}
