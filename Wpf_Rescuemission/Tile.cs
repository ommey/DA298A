using System.Diagnostics;

namespace Wpf_Rescuemission
{
    // Julia Tadic 2024-11-17

    // Enum for describing the direction of walls surrounding a tile
    public enum WallDirection
    {
        None = 0,
        Top = 1,
        Bottom = 2,
        Left = 4,
        Right = 8
    }

    // Class for the Tile
    public class Tile
    {
        // Define the delegate for the event
        public delegate void TileEventHandler(object sender, TileEventArgs args);

        // Event that other classes can subscribe to
        public event TileEventHandler TileChanged;
        public event TileEventHandler ObjectAdded;
        
        private bool _isFire;
        private bool _isSmoke;
        private bool _isMaterial;
        private bool _isPerson;
        private bool _hasFirefighter;

        // List for the image paths to be displayed on the tile
        public List<String> ImagePaths = new List<string>();
        public WallDirection Walls { get; set; }
        public int Row { get; } 
        public int Column { get; }


        // Constructor for the Tile
        public Tile(int row, int column)
        {
            Row = row;
            Column = column;
            _isFire = false;
            _isSmoke = false;
            _isMaterial = false;
            _isPerson = false;
        }

        // Set/get for IsFire property, with event
        public bool IsFire
        {
            get { return _isFire; }
            set
            {
                if (_isFire != value) // Check if the value actually changes
                {
                    _isFire = value;
                    if (_isFire)
                    {
                        ImagePaths.Add("Pictures\\fire.png"); // Add the fire image to the list
                        ObjectAdded?.Invoke(this, new TileEventArgs("all:Fire ", this)); 
                    }
                    else
                    {
                        ImagePaths.Remove("Pictures\\fire.png"); // Remove the fire image from the list
                    }
                    TileChanged?.Invoke(this, new TileEventArgs(" ", this)); // Invoke the event
                }
            }
        }

        // Set/get for IsSmoke property, with event
        public bool IsSmoke
        {
            get { return _isSmoke; }
            set
            {
                if (_isSmoke != value) // Check if the value actually changes
                {
                    _isSmoke = value;
                    if (_isSmoke)
                    {
                        ImagePaths.Add("Pictures\\smoke.png"); // Add the smoke image to the list
                        ObjectAdded?.Invoke(this, new TileEventArgs("all:Smoke ", this));
                    }
                    else
                    {
                        Debug.WriteLine("Gick att ta bort smoke? " + ImagePaths.Remove("Pictures\\smoke.png")); // Remove the smoke image from the list
                    }
                    TileChanged?.Invoke(this, new TileEventArgs(" ", this)); // Invoke the event 
                }
            }
        }

        // Set/get for IsMaterial property, with event
        public bool IsMaterial
        {
            get { return _isMaterial; }
            set
            {
                if (_isMaterial != value) // Check if the value actually changes
                {
                    _isMaterial = value;
                    if (_isMaterial)
                    {
                        ImagePaths.Add("Pictures\\hazmat.png"); // Add the hazmat image to the list
                        ObjectAdded?.Invoke(this, new TileEventArgs("all:Hazmat ", this));
                    }
                    else
                    {
                        ImagePaths.Remove("Pictures\\hazmat.png"); // Remove the hazmat image from the list
                    }
                    TileChanged?.Invoke(this, new TileEventArgs(" ", this)); // Invoke the event 
                }
            }
        }

        // Set/get for IsPerson property, with event
        public bool IsPerson
        {
            get { return _isPerson; }
            set
            {
                if (_isPerson != value) // Check if the value actually changes
                {
                    _isPerson = value;
                    if (_isPerson)
                    {
                        ImagePaths.Add("Pictures\\POI_3.png"); // Add the person image to the list
                        ObjectAdded?.Invoke(this, new TileEventArgs("all:Victim ", this));
                    }
                    else
                    {
                        ImagePaths.Remove("Pictures\\POI_3.png"); // Remove the person image from the list
                    }
                    TileChanged?.Invoke(this, new TileEventArgs(" ", this)); // Invoke the event 
                }
            }
        }


        // Set/get for HasFirefighter property
        public bool HasFirefighter
        {
            get { return _hasFirefighter; }
            set
            {
                _hasFirefighter = value;

                if (_hasFirefighter)
                {
                    ImagePaths.Add("Pictures\\pngegg.png"); // Lägg till bilden
                }
                else if (!_hasFirefighter && ImagePaths.Contains("Pictures\\pngegg.png"))
                {
                    ImagePaths.Remove("Pictures\\pngegg.png"); // Ta bort bilden
                }

                TileChanged?.Invoke(this, new TileEventArgs(" ", this));
            }
        }
    }
}
