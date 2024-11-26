using System;
using System.Diagnostics;
using System.Windows.Media.Media3D;

namespace Wpf_Rescuemission
{
    // Julia Tadic 2024-11-17

    // Enum for the test type
    public enum Test
    {
        MAIN,   // Main test generate 3 fires, 3 persons and 2 materials randomly
        FIRE,   // Test with 1 fires
        VICTIM, // Test with 1 person
        HAZMAT, // Test with 1 material
        SMOKE   // Test with 1 smoke
    }

    // Class for the TileGrid
    public class TileGrid
    {
        // Define an array of Tile-objekt
        public Tile[,] tiles;
        // Create a random object
        private Random random = new Random();
        // Define the number of rows and columns
        private int rows, columns;
        // Define the test type
        public Test test;

        public delegate void TileGridEventHandler(object sender, TileEventArgs arg);
        public event TileGridEventHandler FirefighterDead;
        public event TileGridEventHandler VictimDead;
        public event TileGridEventHandler LoggUpdate; 

        // Constructor for the TileGrid
        public TileGrid(int rows = 6, int columns = 8, Test test = Test.MAIN)
        {
            this.rows = rows;
            this.columns = columns;
            this.test = test;
            tiles = new Tile[rows, columns];
            InitializeGrid();
            AddWalls();
        }

        // Initialize the grid with tiles
        private void InitializeGrid()
        {
             for (int r = 0; r < rows; r++)
             {
                 for (int c = 0; c < columns; c++)
                 {
                     tiles[r, c] = new Tile(r, c);
                 }
             }
        }

        // Setup the grid with walls, fire, persons and materials
        public void Setup()
        {
            switch (test)
            {
                case Test.MAIN:
                    AddFire();
                    //AddPersons();
                    AddMaterial();
                    break;
                case Test.FIRE:
                    tiles[1, 5].IsFire = true;
                    tiles[2, 5].IsFire = true;
                    tiles[3, 5].IsFire = true;
                    tiles[2, 4].IsFire = true;
                    tiles[2, 6].IsFire = true;
                    break;
                case Test.VICTIM:
                    tiles[5, 4].IsPerson = true;
                    break;
                case Test.SMOKE:
                    tiles[4, 3].IsSmoke = true;
                    break;
                case Test.HAZMAT:
                    tiles[5, 1].IsMaterial = true;
                    break;
            }
        }

        // Add walls to the grid using the flash point industry layout 
        private void AddWalls()
        {
            tiles[0, 0].Walls = WallDirection.Top | WallDirection.Left;
            tiles[1, 0].Walls = WallDirection.Left;
            tiles[2, 0].Walls = WallDirection.Left | WallDirection.Bottom;
            tiles[3, 0].Walls = WallDirection.Left | WallDirection.Top;
            tiles[4, 0].Walls = WallDirection.Left;
            tiles[5, 0].Walls = WallDirection.Bottom | WallDirection.Right | WallDirection.Left;
            tiles[0, 1].Walls = WallDirection.Top;
            tiles[2, 1].Walls = WallDirection.Bottom;
            tiles[3, 1].Walls = WallDirection.Top | WallDirection.Right;
            tiles[4, 1].Walls = WallDirection.Bottom | WallDirection.Right;
            tiles[5, 1].Walls = WallDirection.Bottom | WallDirection.Left | WallDirection.Top;
            tiles[0, 2].Walls = WallDirection.Top | WallDirection.Right;
            tiles[1, 2].Walls = WallDirection.Right;
            tiles[2, 2].Walls = WallDirection.Right | WallDirection.Bottom;
            tiles[3, 2].Walls = WallDirection.Right | WallDirection.Top | WallDirection.Left;
            tiles[4, 2].Walls = WallDirection.Right | WallDirection.Left;
            tiles[5, 2].Walls = WallDirection.Bottom | WallDirection.Right;
            tiles[0, 3].Walls = WallDirection.Left | WallDirection.Right | WallDirection.Top;
            tiles[1, 3].Walls = WallDirection.Left | WallDirection.Right;
            tiles[2, 3].Walls = WallDirection.Left | WallDirection.Right;
            tiles[3, 3].Walls = WallDirection.Left | WallDirection.Right;
            tiles[4, 3].Walls = WallDirection.Bottom | WallDirection.Left | WallDirection.Right;
            tiles[5, 3].Walls = WallDirection.Bottom | WallDirection.Right | WallDirection.Top | WallDirection.Left;
            tiles[0, 4].Walls = WallDirection.Top | WallDirection.Left;
            tiles[1, 4].Walls = WallDirection.Left;
            tiles[2, 4].Walls = WallDirection.Left;
            tiles[3, 4].Walls = WallDirection.Left;
            tiles[4, 4].Walls = WallDirection.Left | WallDirection.Bottom;
            tiles[5, 4].Walls = WallDirection.Right | WallDirection.Bottom | WallDirection.Left | WallDirection.Top;
            tiles[0, 5].Walls = WallDirection.Top;
            tiles[5, 5].Walls = WallDirection.Left | WallDirection.Right | WallDirection.Bottom;
            tiles[0, 6].Walls = WallDirection.Top | WallDirection.Right;
            tiles[1, 6].Walls = WallDirection.Right;
            tiles[2, 6].Walls = WallDirection.Right;
            tiles[3, 6].Walls = WallDirection.Right;
            tiles[4, 6].Walls = WallDirection.Right;
            tiles[5, 6].Walls = WallDirection.Right | WallDirection.Bottom | WallDirection.Left;
            tiles[0, 7].Walls = WallDirection.Top | WallDirection.Right | WallDirection.Left;
            tiles[1, 7].Walls = WallDirection.Right | WallDirection.Bottom | WallDirection.Left;
            tiles[2, 7].Walls = WallDirection.Top | WallDirection.Right | WallDirection.Left;
            tiles[3, 7].Walls = WallDirection.Bottom | WallDirection.Right | WallDirection.Left;
            tiles[4, 7].Walls = WallDirection.Top | WallDirection.Right | WallDirection.Left;
            tiles[5, 7].Walls = WallDirection.Right | WallDirection.Bottom | WallDirection.Left;
        }

        // Add fire randomly to the grid 
        private void AddFire()
        {
            int added = 0;
            while (added < 3)
            {
                Tile tile = GetRandomTile();

                if (!tile.IsFire && !tile.IsPerson && !tile.IsMaterial && tile != tiles[0, 3])
                {
                    tile.IsFire = true;
                    Explosion(tile);
                    added++;
                }
            }
        }

        // Check if the row and column is within the grid
        private bool InRange(int row, int column)
        {
            if (row < rows && row >= 0 && column < columns && column >= 0)
            {
                return true; 
            }
            return false; 
        }

        // Add persons randomly to the grid
        private void AddPersons()
        {
            AddRandomElements((tile) => tile.IsPerson = true, 3, (tile) => !tile.IsFire && !tile.IsPerson && !tile.IsMaterial);
        }

        // Add materials randomly to the grid
        private void AddMaterial()
        {
            AddRandomElements((tile) => tile.IsMaterial = true, 2, (tile) => !tile.IsFire && !tile.IsPerson && !tile.IsMaterial);
        }

        // Add random elements to the grid
        private void AddRandomElements(Action<Tile> elementSetter, int count, Func<Tile, bool> isValid)
        {
            int added = 0;
            while (added < count)
            {
                Tile tile = GetRandomTile();

                if (tile != null && isValid(tile) && tile != tiles[0,3])
                {
                    elementSetter(tile);
                    added++;
                }
            }
        }

        // Get a random tile from the grid
        private Tile GetRandomTile()
        {
            int random_column = random.Next(0, columns);
            int random_row = random.Next(0, rows);
            return tiles[random_row, random_column];
        }

        // Updates for each fifth round 
        public string FifthRoundUpdate()
        {
            Tile tile = GetRandomTile();
            String message = "Fith Round Update:";

            if (tile.IsFire)
            {
                Explosion(tile);
                message += "\n\t\tExplosion at random tile";
            }
            else if (tile.IsMaterial)
            {
                Explosion(tile);
                message += "\n\t\tExplosion at random tile";
            }
            else if (tile.IsSmoke)
            {
                tile.IsFire = true;
                tile.IsSmoke = false;
                message += "\n\t\tFire started from smoke at random tile";
            }
            else
            {
                tile.IsSmoke = true;
                message += "\n\t\tSmoke started at random tile";
            }

            foreach(Tile _tile in tiles)
            {
                if (_tile.IsSmoke)
                {
                    bool startFire = false;

                    if ((_tile.Walls & WallDirection.Left) == 0 && InRange(_tile.Row, _tile.Column - 1))
                    {
                        if (tiles[_tile.Row, _tile.Column - 1].IsFire)
                        {
                            startFire = true;
                        }
                    }
                    if ((tile.Walls & WallDirection.Right) == 0 && InRange(tile.Row, tile.Column + 1))
                    {
                        if (tiles[tile.Row, tile.Column + 1].IsFire)
                        {
                            startFire = true;
                        }
                    }
                    if ((tile.Walls & WallDirection.Top) == 0 && InRange(tile.Row - 1, tile.Row))
                    {
                        if (tiles[tile.Row - 1, tile.Column].IsFire)
                        {
                            startFire = true;
                        }
                    }
                    if ((tile.Walls & WallDirection.Bottom) == 0 && InRange(tile.Row + 1, tile.Column))
                    {
                        if (tiles[tile.Row + 1, tile.Column].IsFire)
                        {
                            startFire = true;
                        }
                    }

                    if (startFire)
                    {
                        _tile.IsSmoke = false;
                        message += "\n\t\tFire started from smoke";
                        _tile.IsFire = true;
                    }
                }
            }

            foreach(Tile _tile_ in tiles)
            {
                if (_tile_.IsFire)
                {
                    if (_tile_.IsPerson)
                    {
                        _tile_.IsPerson = false;
                         message += "\n\t\tPerson died in fire";
                        VictimDead?.Invoke(this, new TileEventArgs("Victim dead", _tile_));
                    }
                    if (_tile_.HasFirefighter)
                    {
                        _tile_.HasFirefighter = false;
                        _tile_.IsPerson = true;
                        _tile_.IsFire = false;
                        message += "\n\t\tFirefighter died in fire";
                        FirefighterDead?.Invoke(this, new TileEventArgs("Firefighter dead", _tile_));
                    }
                }
            }
            return message;
        }

        // Explosion method that spreads the fire
        private void Explosion(Tile tile)
        {
            if ((tile.Walls & WallDirection.Left) == 0 && InRange(tile.Row, tile.Column - 1))
            {
                tiles[tile.Row, tile.Column - 1].IsFire = true;
            }
            if ((tile.Walls & WallDirection.Right) == 0 && InRange(tile.Row, tile.Column + 1))
            {
                tiles[tile.Row, tile.Column + 1].IsFire = true;
            }
            if ((tile.Walls & WallDirection.Top) == 0 && InRange(tile.Row - 1, tile.Row))
            {
                tiles[tile.Row - 1, tile.Column].IsFire = true;
            }
            if ((tile.Walls & WallDirection.Bottom) == 0 && InRange(tile.Row + 1, tile.Column))
            {
                tiles[tile.Row + 1, tile.Column].IsFire = true;
            }
        }
        
        public void HandlePutout(string[] tokens)
        {
            Tile tile; 
            if (GetTileFromTokens(tokens[2], tokens[3], out tile))
            {
                if (tokens[0] == "Fire")
                {
                    tile.IsFire = false;
                    tile.IsSmoke = true;
                    LoggUpdate?.Invoke(this, new TileEventArgs("Fire has been put out at tile " + tile.Row + ", " + tile.Column, tile));
                    return; 
                }
                else if (tokens[0] == "Smoke")
                {
                    tile.IsSmoke = false;
                    LoggUpdate?.Invoke(this, new TileEventArgs("Smoke has been put out at tile " + tile.Row + ", " + tile.Column, tile));
                    return; 
                }
            }
            LoggUpdate?.Invoke(this, new TileEventArgs("Putout request had the wrong formatt!", tile));
        }

        public void HandleChangedPosition(string[] tokens)
        {
            Tile fromTile;
            Tile toTile;
            Debug.WriteLine(tokens[0]);
            Debug.WriteLine(tokens[1]);
            Debug.WriteLine(tokens[2]);
            Debug.WriteLine(tokens[3]);
            Debug.WriteLine(tokens[4]);
            Debug.WriteLine(tokens[5]);
            Debug.WriteLine(tokens[6]);



            if (GetTileFromTokens(tokens[2],tokens[3], out fromTile) && GetTileFromTokens(tokens[5], tokens[6], out toTile))
            {
                switch (tokens[0])
                {
                    case "Hazmat":
                        fromTile.IsMaterial = false;
                        toTile.IsMaterial = true;
                        LoggUpdate?.Invoke(this, new TileEventArgs("Hazmat has been moved from " + fromTile.Row + ", " + fromTile.Column + " to " + toTile.Row + ", " + toTile.Column, toTile)); 
                        break;
                    case "Victim":
                        fromTile.IsPerson = false;
                        toTile.IsPerson = true;
                        LoggUpdate?.Invoke(this, new TileEventArgs("Victim has been moved from " + fromTile.Row + ", " + fromTile.Column + " to " + toTile.Row + ", " + toTile.Column, toTile));
                        break;
                    case "Firefighter":
                        fromTile.HasFirefighter = false; 
                        toTile.HasFirefighter = true;
                        LoggUpdate?.Invoke(this, new TileEventArgs("Firefighter moved from " + fromTile.Row + ", " + fromTile.Column + " to " + toTile.Row + ", " + toTile.Column, toTile));
                        break;
                    default:
                         LoggUpdate?.Invoke(this, new TileEventArgs("Position message had the wrong formatt", toTile));
                        break;
                }
            }
        }

        public void HandleSaved(string[] tokens)
        {
            Tile tile; 
            if (tokens[0] == "Victim" && GetTileFromTokens(tokens[2], tokens[3], out tile))
            {
                tile.IsPerson = false;
                LoggUpdate?.Invoke(this, new TileEventArgs("A person is saved!", tiles[0,0]));
                return; 
            }
            else if (tokens[0] == "Hazmat" && GetTileFromTokens(tokens[2], tokens[3], out tile))
            {
                tile.IsMaterial = false;
                LoggUpdate?.Invoke(this, new TileEventArgs("Hazmat removed from buildning!", tiles[0, 0]));
                return;
            }
        }

        private bool GetTileFromTokens(string row, string column, out Tile tile)
        {
            try
            {
                int R = int.Parse(row);
                int C = int.Parse(column);
                Debug.WriteLine("Row: " + R + " Coumn: " + C);
                tile = tiles[R, C];
                return true;
            }
            catch (FormatException)
            {
                tile = tiles[0, 0];
                return false; 
            }            
        }
    }
}