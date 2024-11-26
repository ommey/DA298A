using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO.Ports;
using System.Diagnostics;
using System.IO;
using System.Data.Common;
using System.Windows.Threading;

namespace Wpf_Rescuemission
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private SerialPort port;
        private TileGrid tileGrid;
        private Image playImage;
        private Image pauseImage;
        private bool IsPaused = false;
        private Test test;
        private DispatcherTimer timer;
        private TimeSpan timeElapsed;
        private bool isPaused = true;
        private int Round = 1;
        private string RoundText = "Round 1: ";

        public MainWindow()
        {
            tileGrid = new TileGrid();
            tileGrid.test= Test.MAIN;
            tileGrid.FirefighterDead += HandleFirefighterDead;
            tileGrid.VictimDead += HandleVictimDead; 
            tileGrid.LoggUpdate += UpdateLogg; 

            InitializeTileEventHandlers();
            InitializeComponent();
            InitializeSerialPort(); 
            PrintGridCoordinates();
            tileGrid.Setup();
            timer = new DispatcherTimer();
            timer.Interval = TimeSpan.FromSeconds(1); 
            timer.Tick += Timer_Tick;
            timeElapsed = TimeSpan.Zero;

            playImage = new Image
            {
                Source = new BitmapImage(new Uri("pack://application:,,,/Pictures/playButton.png")),
                Stretch = Stretch.Uniform,
                Width = 100,
                Height = 100,
            };

            pauseImage = new Image
            {
                Source = new BitmapImage(new Uri("pack://application:,,,/Pictures/pauseButton.png")),
                Stretch = Stretch.Uniform,
                Width = 107,
                Height = 107,
            };
            IsPaused = true;
            PausePlayButton.Content = playImage;
            TickButton.IsEnabled = true;
            InfoLabel.Content = "The simulation is paused";
            Logg.Text = "Simulation started\n";
            FireButton.BorderBrush = Brushes.Transparent;
            VictimButton.BorderBrush = Brushes.Transparent;
            HazmatButton.BorderBrush = Brushes.Transparent;
            SmokeButton.BorderBrush = Brushes.Transparent;
            timer.Start();
        }

        private void HandleFirefighterDead(object sender, TileEventArgs args)
        {
            SendDataToPort("all:Firefighter dead " + args.Tile.Row + " " + args.Tile.Column);             
        }

        private void HandleVictimDead(object sender, TileEventArgs args)
        {
            SendDataToPort("all:Victim dead " + args.Tile.Row + " " + args.Tile.Column);
        }

        private void InitializeTileEventHandlers()
        {
            foreach (Tile tile in tileGrid.tiles)
            {
                tile.TileChanged += TileChangedHandler;
                tile.ObjectAdded += HandleObjectAdded; 
            }
        }

        private void InitializeSerialPort()
        {
            port = new SerialPort
            {
                PortName = "COM6", // Ange rätt COM-port
                BaudRate = 115200,  // Ange baudrate
                Parity = Parity.None,
                DataBits = 8,
                StopBits = StopBits.One,
                Handshake = Handshake.None
            };

            port.DataReceived += SerialPort_DataReceived;

            // Öppna porten
            try
            {
                port.Open();
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Kunde inte öppna porten: {ex.Message}");
            }
        }

        private void SerialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            string data = port.ReadLine();
            Dispatcher.Invoke(() => HandleReceivedData(data));
        }

        public void UpdateLogg(object sender, TileEventArgs args)
        {
            Logg.Text = $"{RoundText}\t{timeElapsed:mm\\:ss}\t{args.Message}\n{Logg.Text}";
        }

        public void UpdateLogg(string info)
        {
            Logg.Text = $"{RoundText}\t{timeElapsed:mm\\:ss}\t{info}\n{Logg.Text}";
        }

        private void HandleReceivedData(string data)
        {
            string[] tokens = data.Split(' ');

            if (tokens.Length > 1)
            {
                switch (tokens[1])
                {
                    case "putout":
                        tileGrid.HandlePutout(tokens);
                        break;
                    case "from":
                        tileGrid.HandleChangedPosition(tokens); 
                        break;
                    case "saved":
                        tileGrid.HandleSaved(tokens);
                        break;
                }
            }
        }

        private void SendDataToPort(string data)
        {
            if (port != null && port.IsOpen)
            {
                try
                {
                    port.WriteLine(data); 
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"Kunde inte skicka data: {ex.Message}");
                }
            }
            else
            {
                MessageBox.Show("Porten är inte öppen.");
            }
        }

        private void AddImageToGrid(string imagePath, int row, int column, HorizontalAlignment hAlign, VerticalAlignment vAlign)
        {
            try
            {
                Image img = new Image
                {
                    Source = new BitmapImage(new Uri(imagePath, UriKind.RelativeOrAbsolute)),
                    Stretch = Stretch.Uniform,
                    Width = 50, 
                    Height = 50,
                    HorizontalAlignment = hAlign, 
                    VerticalAlignment = vAlign    
                };

                Grid.SetRow(img, row);
                Grid.SetColumn(img, column);

                GameGrid.Children.Add(img);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Fel vid inläsning av bilden: {ex.Message}");
            }
        }

        private void PrintGridCoordinates()
        {
            // Antalet rader och kolumner
            int rowCount = GameGrid.RowDefinitions.Count;
            int columnCount = GameGrid.ColumnDefinitions.Count;

            // Iterera över rader och kolumner
            for (int row = 0; row < rowCount; row++)
            {
                for (int col = 0; col < columnCount; col++)
                {
                    Tile tile = tileGrid.tiles[row, col];
                    string coordinates = $"R: {row}, C: {col}";

                    // För att lägga till visuella element
                    var textBlock = new System.Windows.Controls.TextBlock
                    {
                        Text = coordinates,
                        HorizontalAlignment = HorizontalAlignment.Center,
                        VerticalAlignment = VerticalAlignment.Center
                    };

                    // Placera TextBlock i griden
                    Grid.SetRow(textBlock, row);
                    Grid.SetColumn(textBlock, col);
                    GameGrid.Children.Add(textBlock);
                }
            }
        }

        private void AddImage(string imagePath, int row, int column, int count)
        {
            switch (count)
            {
                case 0:
                    AddImageToGrid(imagePath, row, column, HorizontalAlignment.Center, VerticalAlignment.Center);
                    break;
                case 1:
                    AddImageToGrid(imagePath, row, column, HorizontalAlignment.Left, VerticalAlignment.Top);
                    break;
                case 2:
                    AddImageToGrid(imagePath, row, column, HorizontalAlignment.Right, VerticalAlignment.Bottom);
                    break;
                case 3:
                    AddImageToGrid(imagePath, row, column, HorizontalAlignment.Left, VerticalAlignment.Bottom);
                    break;
                case 4:
                    AddImageToGrid(imagePath, row, column, HorizontalAlignment.Right, VerticalAlignment.Top);
                    break;
                case 5:
                    AddImageToGrid(imagePath, row, column, HorizontalAlignment.Left, VerticalAlignment.Center);
                    break;
                case 6:
                    AddImageToGrid(imagePath, row, column, HorizontalAlignment.Right, VerticalAlignment.Center);
                    break;
                case 7:
                    AddImageToGrid(imagePath, row, column, HorizontalAlignment.Center, VerticalAlignment.Top);
                    break;
                case 8:
                    AddImageToGrid(imagePath, row, column, HorizontalAlignment.Center, VerticalAlignment.Bottom);
                    break;
                default:
                    AddImageToGrid(imagePath, row, column, HorizontalAlignment.Center, VerticalAlignment.Center);
                    break;
            }
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (port != null && port.IsOpen)
            {
                port.Close();
            }
        }

        public void TileChangedHandler(object sender, TileEventArgs e)
        {
            Tile tile = (Tile)sender;
            RemoveImagesFromGrid(tile.Row, tile.Column);
            for (int i = 0; i < tile.ImagePaths.Count(); i++)
            {
                AddImage(tile.ImagePaths[i], tile.Row, tile.Column, i);
            }
        }

        public void HandleObjectAdded(object sender, TileEventArgs e)
        {
            SendDataToPort(e.Message + e.Tile.Row + " " + e.Tile.Column); 
        }

        private void RemoveImagesFromGrid(int row, int column)
        {
            // Samla alla element i den specifika rutan
            var elementsToRemove = new List<UIElement>();

            foreach (UIElement child in GameGrid.Children)
            {
                if (Grid.GetRow(child) == row && Grid.GetColumn(child) == column)
                {
                    elementsToRemove.Add(child);
                }
            }

            // Ta bort alla element som hittades
            foreach (var element in elementsToRemove)
            {
                GameGrid.Children.Remove(element);
            }
        }

        private void Window_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                TickButton_Click(sender, e);
            }
            else if (e.Key == Key.Space)
            {
                PausePlayButton_Click(sender, e);
            }
        }

        private void TickButton_Click(object sender, RoutedEventArgs e)
        {
            if (TickButton.IsEnabled)
            {
                Logg.Text = $"{RoundText}\t{timeElapsed:mm\\:ss}\tTick\n{Logg.Text}";
                timeElapsed = timeElapsed.Add(TimeSpan.FromSeconds(1));
                TimeLabel.Content = RoundText + timeElapsed.ToString("mm\\:ss");
                SendDataToPort("all:Tick");

                if (timeElapsed.TotalSeconds % 60 == 0)
                {
                    PerformAction();
                }
            }
        }

        private void PausePlayButton_Click(object sender, RoutedEventArgs e)
        {         
            if (IsPaused)
            {
                IsPaused = false;
                PausePlayButton.Content = pauseImage;
                TickButton.IsEnabled = false;
                InfoLabel.Content = "The simulation is running...";
                Logg.Text = $"{RoundText}\t{timeElapsed:mm\\:ss}\tSimulation continued\n{Logg.Text}";
            }
            else
            {
                IsPaused = true;
                PausePlayButton.Content = playImage;
                TickButton.IsEnabled = true;
                InfoLabel.Content = "The simulation is paused... and the tick button enabled";
                Logg.Text = $"{RoundText}\t{timeElapsed:mm\\:ss}\tSimulation paused\n{Logg.Text}";
            }
        }

        private void MainButton_Click(object sender, RoutedEventArgs e)
        {
            if (tileGrid.test == Test.FIRE)
            {
                FireButton.BorderBrush = Brushes.Transparent;
            }
            if (tileGrid.test == Test.VICTIM)
            {
                VictimButton.BorderBrush = Brushes.Transparent;
            }
            if (tileGrid.test == Test.HAZMAT)
            {
                HazmatButton.BorderBrush = Brushes.Transparent;
            }
            if (tileGrid.test == Test.SMOKE)
            {
                SmokeButton.BorderBrush = Brushes.Transparent;
            }
            MainButton.BorderBrush = Brushes.Gray;
            tileGrid.test = Test.MAIN;
            StartNewTest();
        }

        private void FireButton_Click(object sender, RoutedEventArgs e)
        {
            if (tileGrid.test == Test.MAIN)
            {
                MainButton.BorderBrush = Brushes.Transparent;
            }
            if (tileGrid.test == Test.VICTIM)
            {
                VictimButton.BorderBrush = Brushes.Transparent;
            }
            if (tileGrid.test == Test.HAZMAT)
            {
                HazmatButton.BorderBrush = Brushes.Transparent;
            }
            if (tileGrid.test == Test.SMOKE)
            {
                SmokeButton.BorderBrush = Brushes.Transparent;
            }
            FireButton.BorderBrush = Brushes.Gray;
            tileGrid.test = Test.FIRE;
            StartNewTest();
        }

        private void VictimButton_Click(object sender, RoutedEventArgs e)
        {
            if (tileGrid.test == Test.FIRE)
            {
                FireButton.BorderBrush = Brushes.Transparent;
            }
            if (tileGrid.test == Test.MAIN)
            {
                MainButton.BorderBrush = Brushes.Transparent;
            }
            if (tileGrid.test == Test.HAZMAT)
            {
                HazmatButton.BorderBrush = Brushes.Transparent;
            }
            if (tileGrid.test == Test.SMOKE)
            {
                SmokeButton.BorderBrush = Brushes.Transparent;
            }
            VictimButton.BorderBrush = Brushes.Gray;
            tileGrid.test = Test.VICTIM;
            StartNewTest();
        }

        private void HazmatButton_Click(object sender, RoutedEventArgs e)
        {
            if (tileGrid.test == Test.FIRE)
            {
                FireButton.BorderBrush = Brushes.Transparent;
            }
            if (tileGrid.test == Test.VICTIM)
            {
                VictimButton.BorderBrush = Brushes.Transparent;
            }
            if (tileGrid.test == Test.MAIN)
            {
                MainButton.BorderBrush = Brushes.Transparent;
            }
            if (tileGrid.test == Test.SMOKE)
            {
                SmokeButton.BorderBrush = Brushes.Transparent;
            }
            HazmatButton.BorderBrush = Brushes.Gray;
            tileGrid.test = Test.HAZMAT;
            StartNewTest();
        }

        private void SmokeButton_Click(object sender, RoutedEventArgs e)
        {
            if (tileGrid.test == Test.FIRE)
            {
                FireButton.BorderBrush = Brushes.Transparent;
            }
            if (tileGrid.test == Test.VICTIM)
            {
                VictimButton.BorderBrush = Brushes.Transparent;
            }
            if (tileGrid.test == Test.HAZMAT)
            {
                HazmatButton.BorderBrush = Brushes.Transparent;
            }
            if (tileGrid.test == Test.MAIN)
            {
                MainButton.BorderBrush = Brushes.Transparent;
            }
            SmokeButton.BorderBrush = Brushes.Gray;
            tileGrid.test = Test.SMOKE;
            StartNewTest();
        }

        private void StartNewTest()
        {
            Logg.Clear();
            Logg.Text = "New test started";
            foreach (Tile tile in tileGrid.tiles)
            {
                tile.IsFire = false;
                tile.IsSmoke = false;
                tile.IsMaterial = false;
                tile.IsPerson = false;
                tile.ImagePaths.Clear();
            }
            tileGrid.Setup();
            ResetTimer(); 
        }

        private void Timer_Tick(object sender, EventArgs e)
        {
            if (!IsPaused)
            {
                SendDataToPort("all:Tick");

                timeElapsed = timeElapsed.Add(TimeSpan.FromSeconds(1));

                TimeLabel.Content = RoundText + timeElapsed.ToString("mm\\:ss");

                if (timeElapsed.TotalSeconds % 5 == 0)
                {
                    PerformAction();
                    
                }
            }
        }

        private void ResetTimer()
        {
            timer.Stop();
            timeElapsed = TimeSpan.Zero;
            Round = 0;
            RoundText = "Round 1: ";
            TimeLabel.Content = RoundText + timeElapsed.ToString("mm\\:ss");
            timer.Start();
        }

        private void PerformAction()
        {
            timer.Stop();
            timeElapsed = TimeSpan.Zero;
            Round++; 
            RoundText = "Round " + Round + ": ";
            TimeLabel.Content = RoundText + timeElapsed.ToString("mm\\:ss");
            UpdateLogg(tileGrid.FifthRoundUpdate());            
            timer.Start();
        }
    }
}