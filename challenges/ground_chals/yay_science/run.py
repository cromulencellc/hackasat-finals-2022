import astropoint
import argparse 

def run( args ):
    #try:
    ap = astropoint.AstroPoint( args.interfaces , "science-station", args.game , "challenge.yml")
    ap.run( )
    #except Exception as e:
    #    print( e.message )
    #    print("Error")
    print("Exiting", flush=True )

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--interfaces", default="/home/mike/Finals/has3-digitaltwin/game/local.yml")#required=True)
    parser.add_argument("--game",  default="/home/mike/Finals/has3-digitaltwin/game/game.yml") #required=True)#
    args = parser.parse_args()
    run( args )
