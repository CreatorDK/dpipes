namespace DPipeTestCS
{
    internal class ClientTest
    {
        public static void Run(StartParamsClient parameters)
        {
            List<TestBaseClassConstuctor> clientTestConstructorList = [];

            var assembly = typeof(ServerTest).Assembly;

            foreach (var type in assembly.GetTypes())
            {
                var attributes = type.GetCustomAttributes(true);

                foreach (var attribute in attributes)
                {
                    if (attribute is ClientTestAttribute clientTestAttribute)
                    {
                        var constructorInfo = type.GetConstructor([typeof(string), typeof(string), typeof(string)]);

                        if (constructorInfo != null)
                        {
                            var serverTestConstruct = new TestBaseClassConstuctor(clientTestAttribute.TestName, clientTestAttribute.Title, clientTestAttribute.Description, clientTestAttribute.Enabled, constructorInfo);
                            clientTestConstructorList.Add(serverTestConstruct);
                        }
                    }
                }
            }

            foreach (var testConstructor in clientTestConstructorList)
            {
                if (testConstructor.TestName == parameters.TestName)
                {
                    var test = (ClientTestBaseClass)testConstructor.ConstructorInfo.Invoke([testConstructor.TestName, testConstructor.Title, testConstructor.Description]);

                    try
                    {
                        test.ExecuteWrapper(parameters);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Exception at {test.TestName}: {ex.ToString()}");
                    }
                }
            }
        }
    }
}
