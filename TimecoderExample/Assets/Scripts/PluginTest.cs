using UnityEngine;
using System.Collections;
using System;
using System.Runtime.InteropServices;

public class PluginTest : MonoBehaviour
{
    [DllImport("AudioPluginTimecode")]
    private static extern float GetPitch();

    void Update()
    {
        Debug.Log(GetPitch());
    }
}
