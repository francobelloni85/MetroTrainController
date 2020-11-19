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
        public string GPIO_A = "00000000";

        public string GPIO_B = "00000000";

        public string GPIO_C = "00000000";
              

        public void WriteInput(state state, lever_position lever_position)
        {
            switch (state)
            {

                case state.NORMAL:
                    WritePinInput((int)lever_position);
                    break;
                case state.EMERGENCY:
                    WritePinInput(0);
                    break;
                case state.STOP:
                    WritePinInput(1);
                    break;
            }

        }

        private void WritePinInput(int index)
        {
            if (index > 8 || index < 0)
                throw new System.Exception("Wrong index");

            string empty = "00000000";
            StringBuilder sb = new StringBuilder(empty);
            sb[index] = '1';

            GPIO_B = sb.ToString();

        }

        public void WritePinOutput(int index)
        {
            if (index > 12 || index < 0)
                throw new System.Exception("Wrong index");

            string empty = "000000000000";
            StringBuilder sb = new StringBuilder(empty);
            sb[index] = '1';

            GPIO_C = sb.ToString();
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
            _events.Add(new EventData() { State = state.NORMAL, Lever_Position = lever_position.strong_braking, Delay = 100 });
            _events.Add(new EventData() { State = state.NORMAL, Lever_Position = lever_position.minimum_acceleration, Delay = 100 });
        }

        // The thread procedure performs the task, such as formatting
        // and printing a document.
        public void Run()
        {
            Console.WriteLine("");
            Console.WriteLine("EventSimulator started");
            
            Thread.Sleep(100);

            foreach (var item in this._events)
            {
                Console.WriteLine("");
                Console.WriteLine("------  new input added ------ ");
                Console.WriteLine("[" + item.State + "][" + item.Lever_Position + "]");                
                Console.WriteLine("------------------------------- ");
                Console.WriteLine("");

                this._variables.WriteInput(item.State, item.Lever_Position);                
                Thread.Sleep(item.Delay);
            }

        }
    }

    public class TrainController
    {

        public int tic = 0;

        // State information used in the task.
        private SharedVariables _variables;

        /// <summary>
        /// The value read from the input
        /// </summary>
        private state CurrentState { get; set; }

        /// <summary>
        /// The value read from the input
        /// </summary>
        public lever_position CurrentLeverPosition { get; set; }

        /// <summary>
        /// Is set to true if the emergency button has been pressed
        /// </summary>
        private bool _is_emergency_ON = false;

        /// <summary>
        /// Is set to true if the emergency stop has been pressed
        /// </summary>
        private bool _is_stop_ON = false;


        // The constructor obtains the state information.
        public TrainController(SharedVariables variables)
        {
            this._variables = variables;
        }

        // The thread procedure performs the task, such as formatting
        // and printing a document.
        public void Run()
        {
            try
            {
                Console.WriteLine("");
                Console.WriteLine("TrainController started");

                while (true)
                {
                    ReadInput(this._variables.GPIO_B);

                    PrintGPIOValues();

                    if (_is_emergency_ON)
                    {
                        CurrentState = state.EMERGENCY;
                        CurrentLeverPosition = lever_position.strong_braking;
                        PrintMyStatus();
                        continue;
                    }

                    if (_is_stop_ON)
                    {
                        CurrentState = state.STOP;
                        CurrentLeverPosition = lever_position.medium_braking;
                        ManageStopSignal();
                        PrintMyStatus();
                        continue;
                    }

                    switch (CurrentState)
                    {

                        case state.EMERGENCY:
                            this._is_emergency_ON = true;
                            WriteOutput(lever_position.strong_braking);
                            break;


                        case state.STOP:
                            this._is_stop_ON = true;
                            WriteOutput(lever_position.medium_braking);
                            break;


                        case state.NORMAL:
                            WriteOutput(CurrentLeverPosition);
                            break;
                    }


                    PrintMyStatus();

                    tic++;

                }

            }
            catch (Exception ex) {
                Console.WriteLine("");
                Console.WriteLine("TrainController Exception");
                Console.WriteLine(ex.Message);
            }
           


        }

        private void ReadInput(string value)
        {

            if (value[0] == '1')
            {
                CurrentState = state.EMERGENCY;
                CurrentLeverPosition = lever_position.strong_braking;
                _is_emergency_ON = true;
            }

            if (value[1] == '1')
            {
                CurrentState = state.STOP;
                CurrentLeverPosition = lever_position.medium_braking;
                _is_stop_ON = true;
            }

            if (value[2] == '1')
            {
                CurrentState = state.NORMAL;
                CurrentLeverPosition = lever_position.strong_braking;
            }

            if (value[3] == '1')
            {
                CurrentState = state.NORMAL;
                CurrentLeverPosition = lever_position.medium_braking;
            }

            if (value[4] == '1')
            {
                CurrentState = state.NORMAL;
                CurrentLeverPosition = lever_position.minimum_braking;
            }

            if (value[5] == '1')
            {
                CurrentState = state.NORMAL;
                CurrentLeverPosition = lever_position.no_acceleration;
            }

            if (value[6] == '1')
            {
                CurrentState = state.NORMAL;
                CurrentLeverPosition = lever_position.minimum_acceleration;
            }

            if (value[7] == '1')
            {
                CurrentState = state.NORMAL;
                CurrentLeverPosition = lever_position.medium_acceleration;
            }

            //if (value[8] == '1')
            //{
            //    CurrentState = state.NORMAL;
            //    CurrentLeverPosition = lever_position.maximum_acceleration;
            //}


        }

        /// <summary>
        /// There are 4 braking force levels
        /// These are communicated to the braking systems via pins 8-12 of GPIOC.
        /// --> Pin 8 on means minimum braking force
        /// --> Pin 12 on means maximum braking force
        /// Pins 8-11 are used to report the position of the braking lever.
        /// Pin 12 is used only for emergency braking.
        /// Outputs must be configured as push-pull.
        /// </summary>
        private void WriteOutput(lever_position lever_Position)
        {

            switch (lever_Position)
            {
                case lever_position.minimum_acceleration:
                    this._variables.WritePinOutput(4);
                    break;

                case lever_position.medium_acceleration:
                    this._variables.WritePinOutput(5);
                    break;

                case lever_position.maximum_acceleration:
                    this._variables.WritePinOutput(6);
                    break;

                case lever_position.no_acceleration:
                    this._variables.WritePinOutput(7);
                    break;

                case lever_position.minimum_braking:
                    this._variables.WritePinOutput(8);
                    break;

                case lever_position.medium_braking:
                    this._variables.WritePinOutput(9);
                    break;

                case lever_position.strong_braking:
                    this._variables.WritePinOutput(10);
                    break;

            }




        }

        /// <summary>
        /// 
        /// Outputs: Braking
        /// 
        /// The signal is asynchronous and unpredictable
        /// When this request is received, the train must stop “gently”: 
        /// -> Motor acceleration is set to zero,
        /// -> Brakes are activated at medium force
        /// This request has greater priority than the commands issued by the
        /// driver. Therefore, the position of the lever is ignored.
        /// 
        /// After stopping because of this signal, the train must not restart until the
        /// stop signal is active. When the stop signal is cleared, the train can
        /// resume normal operation. However, to this end it is first necessary that
        /// the control lever is set in position zero.
        ///
        /// Outputs: Acceleration
        /// 
        /// There are 3 power levels, corresponding to the three positive position
        /// of the lever.
        /// These are communicated to the motor via pins 0-2 of GPIOC.
        /// Pin 0 on means minimum power
        /// Pin 2 on means maximum power
        /// Outputs must be configured as push-pull.
        /// </summary>
        private void ManageStopSignal()
        {



        }


        public void PrintGPIOValues()
        {
            Console.WriteLine("");            
            Console.WriteLine("GPIO_A= " + this._variables.GPIO_A);
            Console.WriteLine("GPIO_B= " + this._variables.GPIO_B);
            Console.WriteLine("GPIO_C= " + this._variables.GPIO_C);
        }

        public void PrintMyStatus() {
            Console.WriteLine("state= " + CurrentState.ToString());
            Console.WriteLine("lever_position=" + CurrentLeverPosition.ToString());
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
        }
    }

}
