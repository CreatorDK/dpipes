using System;
using System.Collections.Generic;

namespace DPipeTestCS
{
    internal class ServerTest
    {
        private static void RunServerTest(StartParamsServer parameters)
        {
            List<TestBaseClassConstuctor> serverTestConstructorList = [];

            var assembly = typeof(ServerTest).Assembly;

            foreach (var type in assembly.GetTypes())
            {
                var attributes = type.GetCustomAttributes(true);

                foreach (var attribute in attributes)
                {
                    if (attribute is ServerTestAttribute serverTestAttribute)
                    {
                        var constructorInfo = type.GetConstructor([typeof(string), typeof(string), typeof(string)]);

                        if (constructorInfo != null)
                        {
                            var serverTestConstruct = new TestBaseClassConstuctor(serverTestAttribute.TestName, serverTestAttribute.Title, serverTestAttribute.Description, serverTestAttribute.Enabled, constructorInfo);
                            serverTestConstructorList.Add(serverTestConstruct);
                        }
                    }
                }
            }

            foreach (var testConstructor in serverTestConstructorList)
            {
                if ((parameters.TestName == "all" && testConstructor.Enabled) || testConstructor.TestName == parameters.TestName)
                {
                    var test = (ServerTestBaseClass)testConstructor.ConstructorInfo.Invoke([testConstructor.TestName, testConstructor.Title, testConstructor.Description]);

                    try
                    {
                        test.ExecuteWrapper(parameters);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Exception at {test.TestName}: {ex}");
                    }

                    Console.WriteLine("");
                }
            }
        } 

        public static void Run(StartParamsServer parameters, bool bothPipes = false)
        {
            if (bothPipes)
            {
                parameters.PIPE_TYPE = CreatorDK.IO.DPipes.DPIPE_TYPE.ANONYMUS_PIPE;
                RunServerTest(parameters);
                parameters.PIPE_TYPE = CreatorDK.IO.DPipes.DPIPE_TYPE.NAMED_PIPE;
                RunServerTest(parameters);
            }
            else
            {
                RunServerTest(parameters);
            }

            Console.WriteLine("Press any key to continue...");
            Console.ReadKey();
        }
    }
}
