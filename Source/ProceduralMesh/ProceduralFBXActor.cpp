// Fill out your copyright notice in the Description page of Project Settings.

#include "ProceduralMesh.h"
#include "ProceduralFBXActor.h"
#define SAMPLE_FILENAME "file.fbx"

// Sets default values
AProceduralFBXActor::AProceduralFBXActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	// PrimaryActorTick.bCanEverTick = true;

}

/*
// Called when the game starts or when spawned
void AProceduralFBXActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProceduralFBXActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}
*/

// Generate a lathe by rotating the given polyline
void AProceduralFBXActor::LoadFBX(const TArray<FVector>& InPoints, const int InSegments, TArray<FProceduralMeshTriangle>& OutTriangles)
{
	UE_LOG(LogClass, Log, TEXT("AAProceduralFBXActor::Lathe POINTS %d"), InPoints.Num());

	TArray<FVector> verts;

	// precompute some trig
	float angle = FMath::DegreesToRadians(360.0f / InSegments);
	float sinA = FMath::Sin(angle);
	float cosA = FMath::Cos(angle);

	/*
	This implementation is rotation around the X Axis, other formulas below

	Z Axis Rotation
	x' = x*cos q - y*sin q
	y' = x*sin q + y*cos q
	z' = z

	X Axis Rotation
	y' = y*cos q - z*sin q
	z' = y*sin q + z*cos q
	x' = x

	Y Axis Rotation
	z' = z*cos q - x*sin q
	x' = z*sin q + x*cos q
	y' = y
	*/

	// Working point array, in which we keep the rotated line we draw with
	TArray<FVector> wp;
	for (int i = 0; i < InPoints.Num(); i++)
	{
		wp.Add(InPoints[i]);
	}

	// Add a first and last point on the axis to complete the OutTriangles
	FVector p0(wp[0].X, 0, 0);
	FVector pLast(wp[wp.Num() - 1].X, 0, 0);

	FProceduralMeshTriangle tri;
	// for each segment draw the OutTriangles clockwise for normals pointing out or counterclockwise for the opposite (this here does CW)
	for (int segment = 0; segment<InSegments; segment++)
	{

		for (int i = 0; i<InPoints.Num() - 1; i++)
		{
			FVector p1 = wp[i];
			FVector p2 = wp[i + 1];
			FVector p1r(p1.X, p1.Y*cosA - p1.Z*sinA, p1.Y*sinA + p1.Z*cosA);
			FVector p2r(p2.X, p2.Y*cosA - p2.Z*sinA, p2.Y*sinA + p2.Z*cosA);

			static const FColor Red(255, 51, 51);
			tri.Vertex0.Color = Red;
			tri.Vertex1.Color = Red;
			tri.Vertex2.Color = Red;

			if (i == 0)
			{
				tri.Vertex0.Position = p1;
				tri.Vertex1.Position = p0;
				tri.Vertex2.Position = p1r;
				OutTriangles.Add(tri);
			}

			tri.Vertex0.Position = p1;
			tri.Vertex1.Position = p1r;
			tri.Vertex2.Position = p2;
			OutTriangles.Add(tri);

			tri.Vertex0.Position = p2;
			tri.Vertex1.Position = p1r;
			tri.Vertex2.Position = p2r;
			OutTriangles.Add(tri);

			if (i == InPoints.Num() - 2)
			{
				tri.Vertex0.Position = p2;
				tri.Vertex1.Position = p2r;
				tri.Vertex2.Position = pLast;
				OutTriangles.Add(tri);
				wp[i + 1] = p2r;
			}

			wp[i] = p1r;
		}
	}

	// Create the FBX SDK manager
	FbxManager* lSdkManager = FbxManager::Create();

	// Create an IOSettings object.
	FbxIOSettings * ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);

	// ... Configure the FbxIOSettings object ...

	// Create an importer.
	FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

	// Declare the path and filename of the file containing the scene.
	// In this case, we are assuming the file is in the same directory as the executable.
	const char* lFilename = "file.fbx";

	// Initialize the importer.
	bool lImportStatus = lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings());

	if (!lImportStatus) {
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
		exit(-1);
	}

	// Create a new scene so it can be populated by the imported file.
	FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");

	// Import the contents of the file into the scene.
	lImporter->Import(lScene);

	// The file has been imported; we can get rid of the importer.
	lImporter->Destroy();

	// File format version numbers to be populated.
	int lFileMajor, lFileMinor, lFileRevision;

	// Populate the FBX file format version numbers with the import file.
	lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);


	int numofgeometrys = lScene->GetGeometryCount();

	for (int i = 0; i < numofgeometrys; i++)
	{
		fbxsdk::FbxGeometry* fbxgeo = lScene->GetGeometry(i);

		//	fbxsdk::FbxMesh* fbxmesh = fbxgeo->Get

		if (fbxgeo != 0)
		{
			int numofcontrolpoints = fbxgeo->GetControlPointsCount();
			FbxVector4* controlpoints = fbxgeo->GetControlPoints(0);
			// x[12] = controlpoints[12]->mData[0];

			//Fbxsdk::FbxVector4* pNormalArray;
			//fbxgeo->GetNormals(pNormalArray);
			//fbxgeo->GetNormalsIndices();

			//	fbxgeo->GetNormals();

			//	fbxgeo->get
		}
	}

	/*
	FbxClassId lShaderClassID = lSdkManager->FindFbxFileClass("Shader", "FlatShader");
	for(int i = 0; i < lNode->GetSrcObjectCount(lShaderClassID); i++)
	{
	FbxObject* lObject = lNode->GetSrcObject(lShaderClassID, i);
	}
	*/
}
