using System;

namespace DPipeTestCS
{
    [AttributeUsage(AttributeTargets.Class)]
    public class ClientTestAttribute : Attribute
    {
        public bool Enabled { get; init; }
        public string TestName { get; init; }
        public string Title { get; init; }
        public string Description { get; init; }

        public ClientTestAttribute(string testName, string title, string description, bool enabled)
        {
            TestName = testName;
            Title = title;
            Description = description;
            Enabled = enabled;
        }
    }
}
