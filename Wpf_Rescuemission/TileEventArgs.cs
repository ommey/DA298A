namespace Wpf_Rescuemission
{
    public class TileEventArgs : EventArgs
    {
        public string Message { get; }
        public Tile Tile { get; }

        public TileEventArgs(string message, Tile tile)
        {
            Message = message;
            Tile = tile;
        }
    }
}
