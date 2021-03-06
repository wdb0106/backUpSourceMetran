/******************************************************************************/
//$COMMON.CPP$
//   File Name:  PHASEMGR.CPP
//   Copyright 1997 Inventive Technologies,Inc. All Rights Reserved.
//
//   Purpose: 
//      This file contains the definition of the PhaseMgr class.  The Phase
//      Manager is responsible for managing all of the breath phases.  
//      Breath phase transitions (e.g. from inhalation to exhalation) are 
//      controlled via the Phase Manager.  The PhaseMgr keeps track of the
//      currently active Mandatory, Spontaneous and Exhalation phases for
//      a given ventilation type.  For example, in Volume Vent Type with 
//      Pressure Support on and Flow Triggering active, the mandatory inhalation
//      phase is VcvInhPhase, the spontaneous inhalation phase is PsvInhPhase
//      and the exhalation phase is FlowTrigExhPhase.  If the operator changes
//      the support pressure setting to 0, the spontaneous inhalation phase
//      becomes CpapInhPhase.  PhaseMgr maintains the phase information.
//
//   Interfaces:
//      Operations in this class such as ProcessNewVentType() and 
//      ProcessNewTriggerType() are invoked by the MgrBdSetting object whenever
//      setting changes occur that affect which breath phases are used.  
//      Operations in this class also invoke operations in the MgrBdSetting
//      class to retrieve current or pending values of settings.
//
//      The SetCurrentPhasePtr() operation, which is invoked each time a phase
//      change occurs, invokes the End() operation of the currently active 
//      phase and the Start() operation of the new phase.
//
//      At the start of each exhalation, the ProcessNewPhase() operation 
//      interfaces with the currently active mode through the ModeMgr to 
//      update mode dependent alarm status on a regular (breath by breath
//      basis).
//
//      On entry into the Volume Ventilation Type, the ProcessNewVentType()
//      operation invokes VcvInhPhase::ResetEndInhPress() to reset the 
//      end inhalation pressure used for compliance compensation of delivered
//      volume breaths.
//
//   Restrictions:
//      There is only 1 instance of this class.
/******************************************************************************/
#include <ModeMgr.h>
#include <PhaseMgr.h>
#include "VcvInhPhase.h"
#include "MgrBdSetting.h"
#include "PlateauPhase.h"
#include "NonVentPhase.h"
#include "MandPressInhPhase.h"
#include "SupportedInhPhase.h"
#include "FlowTrigExhPhase.h"
#include "PressTrigExhPhase.h"
#include "TriggerMgr.h"
#include "APRV_SupportedInhPhase_InPLow.h"
#include "APRV_SupportedInhPhase_InPHigh.h"
#include "InhTestBreathPhase.h"
#include "InhVABreathPhase.h"
#include "DiagnosticPhase.h"
#include "SelfTestPhase.h"
#include "NCPAP_Phase.h"
#include "APRV_PLowPhase.h"
#include "APRV_ExhPhase_InPHigh.h"
#include "APRV_PHighPhase.h"
#include "APRV_ExhPhase_InPLow.h"

#include "DebugTrace.h"


//  Define statics
PhaseMgr* PhaseMgr::S_Instance = NULL;

/******************************************************************************/
//$COMMON.OPERATION$
//    Operation Name: S_GetInstance()
//
//    Processing:
//      Since only 1 instance of this object should be created, any access
//      to the object comes via this operation.  If any other object wants
//      access to the PhaseMgr object, it invokes PhaseMgr::S_GetInstance(),
//      which returns a pointer to the PhaseMgr object.
//
//      If the object has not already been created, this method instantiates
//      it and returns a pointer to the object.  If the object has already been
//      instantiated, a pointer to the object is returned.
//
//    Input Parameters:
//      None
//
//    Output Parameters:
//      None
//
//    Return Values:
//      PhaseMgr* - pointer to the object instantiated from this class
//
//    Pre-Conditions:
//      None
//
//    Miscellaneous:
//      None
//
//    Requirements:
//
/******************************************************************************/
PhaseMgr* PhaseMgr::S_GetInstance (void)
{
    // If the object has not already been created, create it.
    if(NULL == S_Instance)
    {
        S_Instance = new PhaseMgr ();
        ASSERTION(S_Instance != NULL);
    }

    return (S_Instance);

}   // end S_GetInstance()

/******************************************************************************/
//$COMMON.OPERATION$
//    Operation Name: PhaseMgr()
//
//    Processing: 
//      This operation is the constructor for the PhaseMgr object.  This
//      operation constructs all of the phases and initializes its PhaseList
//      with pointers to each of the specific breath phases.  It also 
//      initializes the CurrentPhaseId to eInitPhase and the CurrentPhasePtr
//      to NULL.
//
//    Input Parameters:
//      None
//
//    Output Parameters:
//      None
//
//    Return Values:
//      None
//
//    Pre-Conditions:
//      None
//
//    Miscellaneous:
//      None
//
//    Requirements:  
//
/******************************************************************************/
PhaseMgr::PhaseMgr (void)
{
    SHORT ii;       // loop control

    // Initialize all pointers so that it can be checked that all pointers 
    // are properly initialized.

    ExhPhaseId = eFlowTrigExhPhase;

    for (ii = 0; ii < eNumSpecificPhases; ii++)
    {
        PhaseList[ii] = NULL;
    }
    PhaseList[eVcvInhPhase] = VcvInhPhase::S_GetInstance();

    PhaseList[eThePlateauPhase] = PlateauPhase::S_GetInstance();

    PhaseList[ePcvInhPhase] = new MandPressInhPhase(ePcvInhPress, eBaselinePress);


    PhaseList[ePsvInhPhase] = new SupportedInhPhase(eSupportPress, eBaselinePress);

    PhaseList[ePsvInhPhase_InAPRV_PLow] = new APRV_SupportedInhPhase_InPLow(eSupportPress, eAPRVLowInhPress);

    PhaseList[ePsvInhPhase_InAPRV_PHigh] = new APRV_SupportedInhPhase_InPHigh(eSupportPress, eAPRVHighInhPress);

    PhaseList[eTheMandInhTestBreathPhase] = new InhTestBreathPhase(ePcvInhPress, eBaselinePress);

    PhaseList[eTheMandInhVABreathPhase] = InhVABreathPhase::S_GetInstance();
#ifdef VA_WITH_PS
    PhaseList[eThePsvVAInhPhase] =new SupportedInhPhase(eSupportPress,
                                                        eBaselinePress); //for new VA
#endif

    PhaseList[eFlowTrigExhPhase] = FlowTrigExhPhase ::S_GetInstance();
    PhaseList[ePressTrigExhPhase] = new PressTrigExhPhase();
    PhaseList[eNoTrigExhPhase] = new ExhPhase();

    PhaseList[eTheNonePhase] = NonVentPhase::S_GetInstance();
    PhaseList[eTheDiagPhase] = DiagnosticPhase::S_GetInstance();
    PhaseList[eTheSelfTestPhase] = new SelfTestPhase();
    PhaseList[eTheNCAPPhase] = NCPAP_Phase::S_GetInstance();

    PhaseList[eTheAPRVLowPhase] = APRV_PLowPhase::S_GetInstance();
    PhaseList[eTheAPRVHighPhase] = APRV_PHighPhase::S_GetInstance();
    PhaseList[eTheAPRVExhPhase_InPHigh] = APRV_ExhPhase_InPHigh::S_GetInstance();
    PhaseList[eTheAPRVExhPhase_InPLow] = APRV_ExhPhase_InPLow::S_GetInstance();
#ifdef HFO_SYSTEM
    PhaseList[eTheHFOPhase] = new HFOPhase();
#endif


    MandInhPhaseId = eTheNonePhase;
    SpontInhPhaseId = eTheNonePhase;
    CurrentVentType = eNoVentType;

    CurrentPhaseId = eInitPhase;
    CurrentTriggerType = eNoTriggerType;
    CurrentPhasePtr = NULL;

    // Make sure that all phase pointers were properly initialized
    DEBUG_MGR_PHASE("Initialize PhaseMgr list(%d) \n\t", eNumSpecificPhases);
    for (ii = 0; ii < eNumSpecificPhases; ii++)
    {
        DEBUG_MGR_PHASE("%d ", ii);
        ASSERTION(PhaseList[ii] != NULL);
    }
    DEBUG_MGR_PHASE("-Finish \n");

}   // end PhaseMgr()

/******************************************************************************/
//$COMMON.OPERATION$
//    Operation Name: ProcessNewVentType()
//
//    Processing: 
//      This operation is invoked by MgrBdSetting whenever the BD Task receives
//      a Ventilation Type batch setting change request from the GUI.  A
//      Ventilation Type change means that all of the specific breath phase
//      IDs must be updated.  
//
//      Based on the new ventilation type, this operation sets up the 
//      MandInhPhaseId, the SpontInhPhaseId and the ExhPhaseId attributes.
//
//    Input Parameters:
//      newVentType - new ventilation type
//
//    Output Parameters:
//      None
//
//    Return Values:
//      None
//
//    Pre-Conditions:
//      None
//
//    Miscellaneous:
//      If the new ventilation type is not Pressure, Volume or Non-Invasive,
//      error handling is performed.
//
//    Requirements:  
//
/******************************************************************************/
void PhaseMgr::ProcessNewVentType (E_VentilationType newVentType)
{
    (VcvInhPhase::S_GetInstance())->InitErrorCompensate();

    if(CurrentVentType == newVentType)
        return;

    // Set up the phase IDs based on the new ventilation type.
    switch(newVentType)
    {
    case eVolumeVentilation:

        // On entry into the Volume Ventilation Type, reset the End
        // Inhalation Pressure sample buffer.  End Inh Press is used to
        // compute compliance compensation for delivered volume breaths.
        // Whenever Volume Vent is entered, reset the inhalation pressure
        // buffer.
        (VcvInhPhase::S_GetInstance())->ResetEndInhPress();
        // When the Vent Type is Volume, all mandatory inhalations are VCV
        MandInhPhaseId = eVcvInhPhase;

        // The Spontaneous and Exhalation phases are common for Volume and
        // Pressure Vent Types; invoke a private method to update those
        // phase IDs
        SetCommonPressVolumePhases();
        break;

    case ePressureVentilation:
        // When the Vent Type is Pressure, all mandatory inhalations are PCV
        MandInhPhaseId = ePcvInhPhase;

        // The Spontaneous and Exhalation phases are common for Volume and
        // Pressure Vent Types; invoke a private method to update those
        // phase IDs
        SetCommonPressVolumePhases();
        break;

    default:
        // Unknown ventilation type
        ASSERTION (eFalse);
        break;
    }

    CurrentVentType = newVentType;

}   // end ProcessNewVentType()

/******************************************************************************/
//$COMMON.OPERATION$
//    Operation Name: ProcessNewTriggerType()
//
//    Processing: 
//      This operation is invoked by MgrBdSetting whenever the BD Task receives
//      a Trigger Type single setting change request from the GUI.  A
//      Trigger Type change means that the exhalation phase ID needs to 
//      be updated.  
//
//      Based on the new trigger type, this operation sets up the 
//      ExhPhaseId attribute.
//
//    Input Parameters:
//      newTriggerType - new trigger type
//
//    Output Parameters:
//      None
//
//    Return Values:
//      None
//
//    Pre-Conditions:
//      This operation should only be called when the operator changes the
//      trigger type via a single setting change.  If the trigger type is
//      changed as part of a batch setting change, it is handled through
//      ProcessNewVentType().
//
//    Miscellaneous:
//      If the new trigger type is not Pressure or Flow, error handling is 
//      performed.
//
//    Requirements:  
//
/******************************************************************************/
void PhaseMgr::ProcessNewTriggerType (E_TriggerType newTriggerType)
{
    if(CurrentTriggerType == newTriggerType)
        return;

    // If the new trigger type is Flow, set the ExhPhaseId to the Flow 
    // Triggering Exh Phase.
    if (newTriggerType == eFlowTrigger)
        ExhPhaseId = eFlowTrigExhPhase;

    // If the new trigger type is Pressure, set the ExhPhaseId to the Pressure
    // Triggering Exh Phase.    
    else if (newTriggerType == ePressureTrigger)
        ExhPhaseId = ePressTrigExhPhase;

    // If the new trigger type is Pressure, set the ExhPhaseId to the Pressure
    // Triggering Exh Phase.    
    else if (newTriggerType == eNoTriggerType)
    {
        ExhPhaseId = eNoTrigExhPhase;
    }

    // If the new trigger type is not flow or pressure or no type, perform error handling.
    else
    {
        ASSERTION (eFalse);
    }

    CurrentTriggerType = newTriggerType;

    // change exh phase because trigger has changed
    if(CurrentPhaseId == eExhPhase)
    {
        ProcessNewPhase(eExhPhase);
    }
}   // end ProcessNewTriggerType()


/******************************************************************************/
//$COMMON.OPERATION$
//    Operation Name: SetCommonPressVolumePhases()
//
//    Processing: 
//      This operation is invoked from ProcessNewVentType() to set the 
//      Spontaneous Inhalation and Exhalation Phase Ids when the Ventilation
//      Type is Pressure or Volume.
//
//      This operation retrieves the pending support pressure and trigger type
//      by invoking MgrBdSetting::GetPendingValue() and uses these values
//      to set up the Spontaneous Inhalation and the Exhalation Phase IDs.
//
//    Input Parameters:
//      None
//
//    Output Parameters:
//      None
//
//    Return Values:
//      None
//
//    Pre-Conditions:
//      This operation assumes that MgrBdSetting has already invoked the
//      SetPendingValue() operations for the Support Pressure and Trigger Type
//      settings before this operation is called.
//
//    Miscellaneous:
//      If the trigger type is not flow or pressure, invoke error handling.
//
//    Requirements:  
//
/******************************************************************************/
void PhaseMgr::SetCommonPressVolumePhases (void)
{
    MgrBdSetting* mgrBdSettingPtr = MgrBdSetting::S_GetInstance();

    SpontInhPhaseId = ePsvInhPhase;

    if (mgrBdSettingPtr->GetPendingValue(eTriggerType) == eFlowTrigger)
    {
        ExhPhaseId = eFlowTrigExhPhase;
    }
    else if (mgrBdSettingPtr->GetPendingValue(eTriggerType) == ePressureTrigger)
    {
        ExhPhaseId = ePressTrigExhPhase;
    }
    else if (mgrBdSettingPtr->GetPendingValue(eTriggerType) == eNoTriggerType)
    {
        ExhPhaseId = eNoTrigExhPhase;
    }
    else
    {
        ASSERTION (eFalse);
    }

}   // end SetCommonPressVolumePhases()

/******************************************************************************/
//$COMMON.OPERATION$
//    Operation Name: ProcessNewPhase()
//
//    Processing: 
//      This operation is called from the mode operations that process the
//      triggers.  When triggers fire, they may cause the mode to initiate
//      a breath phase change by invoking this method.
//
//      Based on the new phase ID passed to this opeation, the CurrentPhaseId
//      is updated and the private SetCurrentPhasePtr() operation is invoked
//      to cause the phase transition.
//
//    Input Parameters:
//      newPhase = ID of the new phase
//
//    Output Parameters:
//      None
//
//    Return Values:
//      None
//
//    Pre-Conditions:
//      None
//
//    Miscellaneous:
//      If the new phase is not one of the expected phase IDs, error handling
//      is performed.
//
//    Requirements:  
//
/******************************************************************************/
void PhaseMgr::ProcessNewPhase (E_PhaseId newPhase)
{
    TriggerMgr* plateauTimeTriggerPtr;
    // Update the current Phase ID and Phase pointer based upon the new phase.
    switch (newPhase)
    {
        case eMandInhPhase:

            // Update the CurrentPhaseId with the general phase ID passed to
            // this operation.


            CurrentPhaseId = newPhase;

            // SetCurrentPhasePtr uses the specific mandatory phase ID to
            // update the phase pointer and to transition to the new phase.
            SetCurrentPhasePtr(MandInhPhaseId);
            break;

        case eSpontInhPhase:
            // Update the CurrentPhaseId with the general phase ID passed to
            // this operation.
            CurrentPhaseId = newPhase;

            // SetCurrentPhasePtr uses the specific spontaneous phase ID to
            // update the phase pointer and to transition to the new phase.
            SetCurrentPhasePtr(ePsvInhPhase);
            break;
        case eSpontInhPhase_InAPRV_PLow:
            CurrentPhaseId = newPhase;
            SetCurrentPhasePtr(ePsvInhPhase_InAPRV_PLow);
            break;

        case eSpontInhPhase_InAPRV_PHigh:
            CurrentPhaseId = newPhase;
            SetCurrentPhasePtr(ePsvInhPhase_InAPRV_PHigh);
            break;

        case ePlateauPhase:
            // Whenver a VCV inhalation ends normally, it is assumed, that
            // ventilation transitions to plateau; it is at this point that
            // a check is made to see if the ventilator goes to plateau or
            // exhalation.

            plateauTimeTriggerPtr = TriggerMgr::S_GetInstance();

            if (plateauTimeTriggerPtr->GetTimePeriod(ePlateauTimeTrigger) != 0)
            {
                // If the plateau time is non-zero, update the CurrentPhaseId
                // with the passed phase ID and transition to the Plateau phase.
                CurrentPhaseId = newPhase;
                SetCurrentPhasePtr(eThePlateauPhase);


            }
            else
            {
                // If the plateau time is zero, update the CurrentPhaseId to
                // be the exhalation phase and call SetCurrentPhasePtr() to
                // transition to exhalation.
                CurrentPhaseId = eExhPhase;
                SetCurrentPhasePtr(ExhPhaseId);
            }
            break;
        case eExhPhase:
            //    case eAPRVExhPhase:
            // Update the CurrentPhaseId to the passed parameter and call
            // SetCurrentPhasePtr() to transition to the specific exhalation
            // phase.
            CurrentPhaseId = newPhase;
            SetCurrentPhasePtr(ExhPhaseId);
            break;

        case eNonVentPhase:
            // Update the CurrentPhaseId to the passed parameter and call
            // SetCurrentPhasePtr() to transition to the specific none vent
            // phase.
            CurrentPhaseId = newPhase;
            SetCurrentPhasePtr(eTheNonePhase);
            break;
        case eDiagnosticPhase:
            // Update the CurrentPhaseId to the passed parameter and call
            // SetCurrentPhasePtr() to transition to the specific none vent
            // phase.
            CurrentPhaseId = newPhase;
            SetCurrentPhasePtr(eTheDiagPhase);
            break;
        case eSelfTestPhase:
            // Update the CurrentPhaseId to the passed parameter and call
            // SetCurrentPhasePtr() to transition to the specific none vent
            // phase.
            CurrentPhaseId = newPhase;
            SetCurrentPhasePtr(eTheSelfTestPhase);
            break;

        case eAPRVLowPhase:
            CurrentPhaseId = newPhase;
            SetCurrentPhasePtr(eTheAPRVLowPhase);
            break;
        case eAPRVHighPhase:
            CurrentPhaseId = newPhase;
            SetCurrentPhasePtr(eTheAPRVHighPhase);
            break;
        case eAPRVExhPhase_InPHigh:
            CurrentPhaseId = newPhase;
            SetCurrentPhasePtr(eTheAPRVExhPhase_InPHigh);
            break;
        case eAPRVExhPhase_InPLow:
            CurrentPhaseId = newPhase;
            SetCurrentPhasePtr(eTheAPRVExhPhase_InPLow);
            break;

#ifdef HFO_SYSTEM
        case eHFOPhase:
            // Update the CurrentPhaseId to the passed parameter and call
            // SetCurrentPhasePtr() to transition to the specific HFO phase.
            CurrentPhaseId = newPhase;
            SetCurrentPhasePtr(eTheHFOPhase);
            break;
#endif
        case eNCPAPPhase:
            CurrentPhaseId = newPhase;
            SetCurrentPhasePtr(eTheNCAPPhase);
            break;
        case eMandInhTestBreathPhase:
            CurrentPhaseId = newPhase;
            SetCurrentPhasePtr(eTheMandInhTestBreathPhase);
            break;
        case eMandInhVABreathPhase:
            CurrentPhaseId = newPhase;
            SetCurrentPhasePtr(eTheMandInhVABreathPhase);
            break;
#ifdef  VA_WITH_PS
        case  ePsvInhVABreathPhase: //for new VA
            CurrentPhaseId = newPhase;
            SetCurrentPhasePtr(eThePsvVAInhPhase);
            break;
#endif
        default:
            // Invalid phase
            ASSERTION(eFalse);
            break;

    }

}   // end ProcessNewPhase()

/******************************************************************************/
//$COMMON.OPERATION$
//    Operation Name: GetCurrentPhaseId()
//
//    Processing:  GetCurrentPhaseId (void)
//
//    Input Parameters:
//      phaseId - specific phase ID for the phase that should be started.
//
//    Output Parameters:
//      None
//
//    Return Values:
//      CurrentPhaseId - current phase ID
//
//    Pre-Conditions:
//      None
//
//    Miscellaneous:
//      None
//
//    Requirements:
//
/******************************************************************************/
E_PhaseId PhaseMgr::GetCurrentPhaseId (void)
{
    return CurrentPhaseId;
}

/******************************************************************************/
//$COMMON.OPERATION$
//    Operation Name: GetCurrentVentType()
//
//    Processing:  Get current vent type
//
//    Input Parameters:
//         NOne
//    Output Parameters:
//      None
//
//    Return Values:
//      CurrentVentType - current vent type ID
//
//    Pre-Conditions:
//      None
//
//    Miscellaneous:
//      None
//
//    Requirements:
//
/******************************************************************************/
E_VentilationType PhaseMgr::GetCurrentVentType (void)
{
    return CurrentVentType;
}

/******************************************************************************/
//$COMMON.OPERATION$
//    Operation Name: SetCommonPressVolumePhases()
//
//    Processing:
//      This operation is invoked from ProcessNewVentType() to set the
//      Spontaneous Inhalation and Exhalation Phase Ids when the Ventilation
//      Type is Pressure or Volume.
//
//      This operation retrieves the trigger type by invoking
//      MgrBdSetting::GetPendingValue() and uses the value
//      to set up the the Exhalation Phase IDs.
//
//    Input Parameters:
//      None
//
//    Output Parameters:
//      None
//
//    Return Values:
//      None
//
//    Pre-Conditions:
//      This operation assumes that MgrBdSetting has already invoked the
//      SetPendingValue() operation for the Trigger Type setting before this
//      operation is called.
//
//    Miscellaneous:
//      If the trigger type is not flow or pressure, invoke error handling.
//
//    Requirements:
//
/******************************************************************************/
void PhaseMgr::SetExhPhaseId(void)
{
    ExhPhaseId = eNoTrigExhPhase;

    MgrBdSetting* mgrBdSettingPtr = MgrBdSetting::S_GetInstance();

    if (mgrBdSettingPtr->GetPendingValue(eTriggerType) == eFlowTrigger)
    {
        ExhPhaseId = eFlowTrigExhPhase;


    }
    else if (mgrBdSettingPtr->GetPendingValue(eTriggerType) == ePressureTrigger)
    {

        ExhPhaseId = ePressTrigExhPhase;

    }
    else
    {
        ASSERTION(eFalse);
    }
}   // end SetCommonPressVolumePhases()

/******************************************************************************/
//$COMMON.OPERATION$
//    Operation Name: GetVCVInhPhaseStartedFlag(void)
//
//    Processing: get VCV Inhalation Phase Start flag
//
//    Input Parameters:
//      None
//
//    Output Parameters:
//      None
//
//    Return Values:
//      VcvInhPhase::S_GetInstance()->GetPhaseStarted();
//
//    Pre-Conditions:
//      None
//
//    Miscellaneous:
//      None
//
//    Requirements:
//
/******************************************************************************/
E_Bool PhaseMgr::GetVCVInhPhaseStartedFlag(void)
{
    return VcvInhPhase::S_GetInstance()->GetPhaseStarted();
}

/******************************************************************************/
//$COMMON.OPERATION$
//    Operation Name: SetVCVInhPhaseStartedFlag(E_Bool value)
//
//    Processing: Set VCV Inhalation Phase Start flag
//
//    Input Parameters:
//      VCV Inhalation Phase Start flag
//
//    Output Parameters:
//      None
//
//    Return Values:
//      None
//
//    Pre-Conditions:
//      None
//
//    Miscellaneous:
//      None
//
//    Requirements:
//
/******************************************************************************/
void PhaseMgr::SetVCVInhPhaseStartedFlag(E_Bool value)
{
    VcvInhPhase::S_GetInstance()->SetPhaseStarted(value);
}

/******************************************************************************/
//$COMMON.OPERATION$
//    Operation Name: ResetVolumeAdjustmentOfVCVInhPhase(void)
//
//    Processing: Reset volume of adjment of VCV Inhalation Phase
//
//    Input Parameters:
//      None
//
//    Output Parameters:
//      None
//
//    Return Values:
//      None
//
//    Pre-Conditions:
//      None
//
//    Miscellaneous:
//      None
//
//    Requirements:
//
/******************************************************************************/
void PhaseMgr::ResetVolumeAdjustmentOfVCVInhPhase (void)
{
    VcvInhPhase::S_GetInstance()->ResetVolumeAdjustment();
}

/******************************************************************************/
//$COMMON.OPERATION$
//    Operation Name: SetCurrentPhasePtr()
//
//    Processing: 
//      This operation invokes the End() operation of the currently active
//      phase to wrap up that breath phase.  It then updates the CurrentPhasePtr
//      to the new breath phase and invokes the Start() operation of the new
//      breath phase to transition to that phase.
//
//    Input Parameters:
//      phaseId - specific phase ID for the phase that should be started.
//
//    Output Parameters:
//      None
//
//    Return Values:
//      None
//
//    Pre-Conditions:
//      None
//
//    Miscellaneous:
//      None
//
//    Requirements:  
//
/******************************************************************************/
void PhaseMgr::SetCurrentPhasePtr (E_SpecificPhaseId phaseId)
{
    if(NULL != CurrentPhasePtr)
    {
        // Leave the last breath phase
        CurrentPhasePtr->End ();
    }

    // Update the CurrentPhasePtr and start the new phase
    CurrentPhasePtr = PhaseList[phaseId];
    CurrentPhasePtr->Start ();

}   // end SetCurrentPhasePtr()

/******************************************************************************/
//$COMMON.OPERATION$
//    Operation Name: ProcessBdCycle()
//
//    Processing: 
//      This operation invokes the ProcessBdCycle() operation of the currently
//      active breath phase.
//
//    Input Parameters:
//      None
//
//    Output Parameters:
//      None
//
//    Return Values:
//      None
//
//    Pre-Conditions:
//      None
//
//    Miscellaneous:
//      If the CurrentPhasePtr attribut is NULL, error handling is performed.
//
//    Requirements:  
//
/******************************************************************************/
void PhaseMgr::ProcessBdCycle (void)
{
    DEBUG_MGR_PHASE("Process Phase \n");
    // CurrentPhasePtr cannot be NULL
    //    PRE_CONDITION(CurrentPhasePtr != NULL);

    // Process the cycle in the current phase.    
    CurrentPhasePtr->ProcessBdCycle ();

}   // end  ProcessBdCycle()

