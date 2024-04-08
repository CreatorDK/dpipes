using System.Reflection;

namespace DPipeTestCS
{
    public class TestBaseClassConstuctor
    {
        public bool Enabled { get; init; }
        public string TestName { get; init; }
        public string Title { get; init; }
        public string Description { get; init; }

        public ConstructorInfo ConstructorInfo { get; init; }

        public TestBaseClassConstuctor(string testName, string title, string description, bool enabled, ConstructorInfo constructorInfo)
        {
            TestName = testName;
            Title = title;
            Description = description;
            Enabled = enabled;
            ConstructorInfo = constructorInfo;
        }
    }
}
