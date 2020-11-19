using System;
using System.Threading;

namespace MetroTrainController
{

    public enum state { STARTUP, NORMAL, STOP, EMERGENCY }

    class Program
    {
         

        static void Main(string[] args)
        {
            SharedVariables sharedVariables = new SharedVariables();

            Console.WriteLine("START PROJECT");

            // Start the event simulator
            EventSimulator simulator = new EventSimulator(sharedVariables);
            Thread thread_simulator = new Thread(new ThreadStart(simulator.Run));
            thread_simulator.Priority = ThreadPriority.Highest;
            thread_simulator.Start();

            Thread.Sleep(100);

            // Start the event simulator
            TrainController trainController = new TrainController(sharedVariables);
            Thread thread_trainController = new Thread(new ThreadStart(trainController.Run));
            thread_trainController.Priority = ThreadPriority.Normal;
            thread_trainController.Start();

            Thread.Sleep(100);

            // Start the event simulator
            ManageComunications manage_comunications = new ManageComunications(sharedVariables);
            Thread thread_manage_comunications = new Thread(new ThreadStart(manage_comunications.Run));
            thread_manage_comunications.Priority = ThreadPriority.Lowest;
            thread_manage_comunications.Start();


            while (true) {
                Thread.Sleep(10000);
            }
                
            Console.WriteLine("END PROJECT");

            
        }
    }

    
    public class SharedVariables {

        public state my_state = state.STARTUP;

    }

    public class EventSimulator
    {
        // State information used in the task.
        private SharedVariables _variables;


        // The constructor obtains the state information.
        public EventSimulator(SharedVariables variables)
        {
            this._variables = variables;
        }

        // The thread procedure performs the task, such as formatting
        // and printing a document.
        public void Run()
        {
            Console.WriteLine("");
            Console.WriteLine("EventSimulator started");
            Console.WriteLine("my_state= " + this._variables.my_state.ToString());
        }
    }



    public class TrainController
    {
        // State information used in the task.
        private SharedVariables _variables;


        // The constructor obtains the state information.
        public TrainController(SharedVariables variables)
        {
            this._variables = variables;
            this._variables.my_state = state.NORMAL;
        }

        // The thread procedure performs the task, such as formatting
        // and printing a document.
        public void Run()
        {
            Console.WriteLine("");
            Console.WriteLine("TrainController started");
            Console.WriteLine("my_state= " + this._variables.my_state.ToString());
        }
    }


    public class ManageComunications
    {
        // State information used in the task.
        private SharedVariables _variables;


        // The constructor obtains the state information.
        public ManageComunications(SharedVariables variables)
        {
            this._variables = variables;
            
        }

        // The thread procedure performs the task, such as formatting
        // and printing a document.
        public void Run()
        {
            Console.WriteLine("");
            Console.WriteLine("ManageComunications started");
            Console.WriteLine("my_state= " + this._variables.my_state.ToString());
        }
    }

}
