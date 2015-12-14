using UnityEngine;
using UnityEngine.Audio;
using System.Collections;

public class TimecodeInput : MonoBehaviour
{
    public string DeviceName = "Ch B Input (Traktor Audio 6 WDM Audio)";

    public int BufferLength = 10;
    public int SampleRate = 44100;

    protected AudioClip _clip;
    protected string _choosenDevice;
    protected int _prevPosition = 0;
    protected int _sampleNum = 0;
    protected bool _recording = false;
    protected int _readHead = 0;

    void Start()
    {
        StartRecording();
    }

    [ContextMenu("Enumarate device")]
    void Enumarate()
    {
        foreach(var device in Microphone.devices)
        {
            Debug.Log(device);
        }
    }

    [ContextMenu("Start")]
    void StartRecording()
    {
        if( !_recording)
        {
            try
            {
                _clip = Microphone.Start(DeviceName, true, BufferLength, SampleRate);
                _sampleNum = BufferLength * SampleRate;
                _recording = true;

                _prevPosition = 0;

                StartCoroutine(ReadData());
            }
            catch (System.Exception e)
            {
                Debug.Log("TimecodeInput : Exception -> " + e.Message);
            }
        }
    }

    IEnumerator ReadData()
    {
        while( _recording )
        {
            int writehead = Microphone.GetPosition(DeviceName);
            int buffsize = 0;

            if( writehead < _readHead )
            {
                buffsize = writehead;
                buffsize += _sampleNum - _readHead;
            }else
            {
                buffsize = writehead - _readHead;
            }

            if (buffsize > 0)
            {
                float[] data = new float[buffsize];
                _clip.GetData(data, _readHead);

                // Process data here
            }

            _readHead = writehead;

            yield return new WaitForSeconds(0.005f);
        }
    }

    void OnDestroy()
    {
        Stop();
    }

    [ContextMenu("Stop")]
    void Stop()
    {
        if( _recording )
        {
            Microphone.End(_choosenDevice);
            _recording = false;

            StopCoroutine(ReadData());
        }
    }
}

