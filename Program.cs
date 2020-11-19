using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;

namespace MetroTrainController
{

    public enum state { STARTUP, NORMAL, STOP, EMERGENCY }

    public enum lever_position
    {
        strong_braking = 1,
        medium_braking,
        minimum_braking,
        no_acceleration,
        minimum_acceleration,
        medium_acceleration,
        maximum_acceleration,
    }

    public enum gpio
    {

        /// <summary>
        /// Receive messages
        /// </summary>
        GPIO_A,

        /// <summary>
        /// Inputs
        /// </summary>
        GPIO_B,

        /// <summary>
        /// Outputs
        /// </summary>
        GPIO_C
    }

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


            while (true)
            {
                Thread.Sleep(10000);
            }

            Console.WriteLine("END PROJECT");


        }
    }

    public class EventData
    {
        public state State { get; set; }

        public lever_position Lever_Position { get; set; }

        public int Delay { get; set; } = 0;
    }

    public class SharedVariables
    {
        private state _state = state.STARTUP;

        private lever_position _lever_position = lever_position.strong_braking;

        public string GPIO_A = "00000000";

        public string GPIO_B = "00000000";

        public string GPIO_C = "00000000";

        #region GET & SET

        public state GetState() { return _state; }

        public void SetState(state value) { _state = value; }

        public lever_position GetLeverPosition() { return _lever_position; }

        public void SetLeverPosition(lever_position value)
        {
            _lever_position = value;
        }

        #endregion

        public void WriteOutput(state state, lever_position lever_position)
        {
            SetState(state);
            SetLeverPosition(lever_position);

            switch (state)
            {

                case state.NORMAL:
                    WritePin(gpio.GPIO_B, (int)lever_position);
                    break;
                case state.EMERGENCY:
                    WritePin(gpio.GPIO_B, 0);
                    break;
                case state.STOP:
                    WritePin(gpio.GPIO_B, 1);
                    break;

            }

        }

        private void WritePin(gpio gpio, int index)
        {
            if (index > 8 || index < 0)
                throw new System.Exception("Wrong index");

            string empty = "00000000";
            StringBuilder sb = new StringBuilder(empty);
            sb[index] = '1';

            switch (gpio)
            {
                case gpio.GPIO_A:
                    GPIO_A = sb.ToString();
                    break;
                case gpio.GPIO_B:
                    GPIO_B = sb.ToString();
                    break;
                case gpio.GPIO_C:
                    GPIO_C = sb.ToString();
                    break;
            }

        }

        public void PrintGPIOValues()
        {
            Console.WriteLine("");
            Console.WriteLine("state= " + _state.ToString() + " lever_position=" + _lever_position.ToString());
            Console.WriteLine("GPIO_A= " + GPIO_A);
            Console.WriteLine("GPIO_B= " + GPIO_B);
            Console.WriteLine("GPIO_C= " + GPIO_C);
        }

    }

    public class EventSimulator
    {
        // State information used in the task.
        private SharedVariables _variables;

        private List<EventData> _events = new List<EventData>();

        // The constructor obtains the state information.
        public EventSimulator(SharedVariables variables)
        {
            this._variables = variables;
            LoadEvents();
        }

        private void LoadEvents()
        {
            _events.Add(new EventData() { State = state.NORMAL, Lever_Position = lever_position.no_acceleration, Delay = 100 });
            _events.Add(new EventData() { State = state.NORMAL, Lever_Position = lever_position.minimum_acceleration, Delay = 100 });
            _events.Add(new EventData() { State = state.NORMAL, Lever_Position = lever_position.medium_acceleration, Delay = 100 });
            _events.Add(new EventData() { State = state.NORMAL, Lever_Position = lever_position.maximum_acceleration, Delay = 100 });
            _events.Add(new EventData() { State = state.NORMAL, Lever_Position = lever_position.minimum_braking, Delay = 100 });
            _events.Add(new EventData() { State = state.NORMAL, Lever_Position = lever_position.medium_braking, Delay = 100 });
            _events.Add(new EventData() { State = state.NORMAL, Lever_Position = lever_position.strong_braking, Delay = 100 });
            _events.Add(new EventData() { State = state.STOP, Lever_Position = lever_position.no_acceleration, Delay = 100 });
            _events.Add(new EventData() { State = state.EMERGENCY, Lever_Position = lever_position.no_acceleration, Delay = 100 });
        }

        // The thread procedure performs the task, such as formatting
        // and printing a document.
        public void Run()
        {
            Console.WriteLine("");
            Console.WriteLine("EventSimulator started");
            Console.WriteLine("my_state= " + this._variables.GetState().ToString());
            Thread.Sleep(100);

            foreach (var item in this._events)
            {
                this._variables.WriteOutput(item.State, item.Lever_Position);
                this._variables.PrintGPIOValues();
                Thread.Sleep(item.Delay);
            }

        }
    }

    public class TrainController
    {
        // State information used in the task.
        private SharedVariables _variables;

        /// <summary>
        /// State actually in the memory of the system
        /// </summary>
        public lever_position CurrentLeverPosition { get; set; }

        // The constructor obtains the state information.
        public TrainController(SharedVariables variables)
        {
            this._variables = variables;
            this._variables.SetState(state.NORMAL);
        }

        // The thread procedure performs the task, such as formatting
        // and printing a document.
        public void Run()
        {
            Console.WriteLine("");
            Console.WriteLine("TrainController started");
            Console.WriteLine("my_state= " + this._variables.GetState().ToString());
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
            Console.WriteLine("my_state= " + this._variables.GetState().ToString());
        }
    }

}
