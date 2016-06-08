#pylint: disable=no-init, too-many-instance-attributes
from mantid import logger, AlgorithmFactory
from mantid.api import *
from mantid.kernel import *
import mantid.simpleapi as ms

class IqtFitSequential(PythonAlgorithm):

    _input_ws = None
    _function = None
    _fit_type = None
    _start_x = None
    _end_x = None
    _spec_min = None
    _spec_max = None
    _intensities_constrained = None
    _minimizer = None
    _max_iterations = None
    _result_name = None
    _result_ws = None
    _parameter_name = None
    _fit_group_name = None


    def category(self):
        return "Workflow\\MIDAS"

    def summary(self):
        #pylint: disable=anomalous-backslash-in-string
        return "Fits an \*\_iqt file generated by I(Q,t) sequentially."

    def PyInit(self):
        self.declareProperty(MatrixWorkspaceProperty('InputWorkspace', '', direction=Direction.Input),
                             doc='The _iqt.nxs InputWorkspace used by the algorithm')

        self.declareProperty(name='Function', defaultValue='',
                             doc='The function to use in fitting')

        self.declareProperty(name='FitType', defaultValue='',
                             doc='The type of fit being carried out')

        self.declareProperty(name='StartX', defaultValue=0.0,
                             validator=FloatBoundedValidator(0.0),
                             doc="The first value for X")

        self.declareProperty(name='EndX', defaultValue=0.2,
                             validator=FloatBoundedValidator(0.0),
                             doc="The last value for X")

        self.declareProperty(name='SpecMin', defaultValue=0,
                             validator=IntBoundedValidator(0),
                             doc='Minimum spectra in the worksapce to fit')

        self.declareProperty(name='SpecMax', defaultValue=1,
                             validator=IntBoundedValidator(0),
                             doc='Maximum spectra in the worksapce to fit')

        self.declareProperty(name='Minimizer', defaultValue='Levenberg-Marquardt',
                             doc='The minimizer to use in fitting')

        self.declareProperty(name="MaxIterations", defaultValue=500,
                             validator=IntBoundedValidator(0),
                             doc="The Maximum number of iterations for the fit")

        self.declareProperty(name='ConstrainIntensities', defaultValue=False,
                             doc="If the Intensities should be constrained during the fit")

        self.declareProperty(MatrixWorkspaceProperty('OutputResultWorkspace', '', direction=Direction.Output),
                             doc='The outputworkspace containing the results of the fit data')

        self.declareProperty(ITableWorkspaceProperty('OutputParameterWorkspace', '', direction=Direction.Output),
                             doc='The outputworkspace containing the parameters for each fit')

        self.declareProperty(WorkspaceGroupProperty('OutputWorkspaceGroup', '', direction=Direction.Output),
                             doc='The OutputWorkspace group Data, Calc and Diff, values for the fit of each spectra')


    def validateInputs(self):
        self._get_properties()
        issues = dict()
        if self._start_x >= self._end_x:
            issues['StartX'] = 'StartX must be more than EndX'
        return issues


    def _get_properties(self):
        self._input_ws = self.getProperty('InputWorkspace').value
        self._function = self.getProperty('Function').value
        self._fit_type = self.getProperty('FitType').value
        self._start_x = self.getProperty('StartX').value
        self._end_x = self.getProperty('EndX').value
        self._spec_min = self.getProperty('SpecMin').value
        self._spec_max = self.getProperty('SpecMax').value
        self._intensities_constrained = self.getProperty('ConstrainIntensities').value
        self._minimizer = self.getProperty('Minimizer').value
        self._max_iterations = self.getProperty('MaxIterations').value
        self._result_name = self.getPropertyValue('OutputResultWorkspace')
        self._parameter_name = self.getPropertyValue('OutputParameterWorkspace')
        self._fit_group_name = self.getPropertyValue('OutputWorkspaceGroup')


    def PyExec(self):
        from IndirectDataAnalysis import (convertToElasticQ)
        from IndirectCommon import (getWSprefix)

        setup_prog = Progress(self, start=0.0, end=0.1, nreports=4)
        self._fit_type = self._fit_type[:-2]
        logger.information('Option: ' + self._fit_type)
        logger.information(self._function)

        setup_prog.report('Cropping workspace')
        tmp_fit_name = "__IqtFit_ws"
        crop_alg = self.createChildAlgorithm("CropWorkspace")
        crop_alg.setProperty("InputWorkspace", self._input_ws)
        crop_alg.setProperty("OutputWorkspace", tmp_fit_name)
        crop_alg.setProperty("XMin", self._start_x)
        crop_alg.setProperty("XMax", self._end_x)
        crop_alg.execute()

        num_hist = self._input_ws.getNumberHistograms()
        if self._spec_max is None:
            self._spec_max = num_hist - 1

        # Name stem for generated workspace
        output_workspace = '%sIqtFit_%s_s%d_to_%d' % (getWSprefix(self._input_ws.getName()),
                                                  self._fit_type, self._spec_min,
                                                  self._spec_max)

        setup_prog.report('Converting to Histogram')
        convert_to_hist_alg = self.createChildAlgorithm("ConvertToHistogram")
        convert_to_hist_alg.setProperty("InputWorkspace", crop_alg.getProperty("OutputWorkspace").value)
        convert_to_hist_alg.setProperty("OutputWorkspace", tmp_fit_name)
        convert_to_hist_alg.execute()
        mtd.addOrReplace(tmp_fit_name, convert_to_hist_alg.getProperty("OutputWorkspace").value)

        setup_prog.report('Convert to Elastic Q')
        convertToElasticQ(tmp_fit_name)

        # Build input string for PlotPeakByLogValue
        input_str = [tmp_fit_name + ',i%d' % i for i in range(self._spec_min, self._spec_max + 1)]
        input_str = ';'.join(input_str)

        fit_prog = Progress(self, start=0.1, end=0.8, nreports=2)
        fit_prog.report('Fitting...')
        ms.PlotPeakByLogValue(Input=input_str,
                              OutputWorkspace=output_workspace,
                              Function=self._function,
                              Minimizer=self._minimizer,
                              MaxIterations=self._max_iterations,
                              StartX=self._start_x,
                              EndX=self._end_x,
                              FitType='Sequential',
                              CreateOutput=True)
        fit_prog.report('Fitting complete')

        conclusion_prog = Progress(self, start=0.8, end=1.0, nreports=5)
        # Remove unsused workspaces
        delete_alg = self.createChildAlgorithm("DeleteWorkspace")
        delete_alg.setProperty("Workspace", output_workspace + '_NormalisedCovarianceMatrices')
        delete_alg.execute()
        delete_alg.setProperty("Workspace", output_workspace + '_Parameters')
        delete_alg.execute()
        delete_alg.setProperty("Workspace", tmp_fit_name)
        delete_alg.execute()

        conclusion_prog.report('Renaming workspaces')
        # rename workspaces to match user input
        rename_alg = self.createChildAlgorithm("RenameWorkspace")
        if output_workspace + "_Workspaces" != self._fit_group_name:
            rename_alg.setProperty("InputWorkspace", output_workspace + "_Workspaces")
            rename_alg.setProperty("OutputWorkspace", self._fit_group_name)
            rename_alg.execute()
        if output_workspace != self._parameter_name:
            rename_alg.setProperty("InputWorkspace", output_workspace)
            rename_alg.setProperty("OutputWorkspace", self._parameter_name)
            rename_alg.execute()

        # Create *_Result workspace
        parameter_names = 'A0,Intensity,Tau,Beta'
        conclusion_prog.report('Processing indirect fit parameters')
        pifp_alg = self.createChildAlgorithm("ProcessIndirectFitParameters")
        pifp_alg.setProperty("InputWorkspace", self._parameter_name)
        pifp_alg.setProperty("ColumnX", "axis-1")
        pifp_alg.setProperty("XAxisUnit", "MomentumTransfer")
        pifp_alg.setProperty("ParameterNames", parameter_names)
        pifp_alg.setProperty("OutputWorkspace", self._result_name)
        pifp_alg.execute()
        self._result_ws = pifp_alg.getProperty("OutputWorkspace").value

        mtd.addOrReplace(self._result_name, self._result_ws)

        # Process generated workspaces
        wsnames = mtd[self._fit_group_name].getNames()
        for i, workspace in enumerate(wsnames):
            output_ws = output_workspace + '_Workspace_%d' % i
            rename_alg.setProperty("InputWorkspace", workspace)
            rename_alg.setProperty("OutputWorkspace", output_ws)
            rename_alg.execute()

        conclusion_prog.report('Copying and transfering sample logs')
        self._transfer_sample_logs()

        self.setProperty('OutputParameterWorkspace', self._parameter_name)
        self.setProperty('OutputWorkspaceGroup', self._fit_group_name)
        self.setProperty('OutputResultWorkspace', self._result_ws)
        conclusion_prog.report('Algorithm complete')


    def _transfer_sample_logs(self):
        """
        Copy the sample logs from the input workspace and add them to the output workspaces
        """

        sample_logs  = {'start_x': self._start_x, 'end_x': self._end_x, 'fit_type': self._fit_type,
                        'intensities_constrained': self._intensities_constrained, 'beta_constrained': False}

        copy_log_alg = self.createChildAlgorithm("CopyLogs")
        copy_log_alg.setProperty("InputWorkspace", self._input_ws)
        copy_log_alg.setProperty("OutputWorkspace", self._fit_group_name)
        copy_log_alg.execute()
        copy_log_alg.setProperty("InputWorkspace", self._input_ws)
        copy_log_alg.setProperty("OutputWorkspace", self._result_ws.getName())
        copy_log_alg.execute()

        log_names = [item for item in sample_logs]
        log_values = [sample_logs[item] for item in sample_logs]

        add_sample_log_multi = self.createChildAlgorithm("AddSampleLogMultiple")
        add_sample_log_multi.setProperty("Workspace", self._result_ws.getName())
        add_sample_log_multi.setProperty("LogNames", log_names)
        add_sample_log_multi.setProperty("LogValues", log_values)
        add_sample_log_multi.execute()
        add_sample_log_multi.setProperty("Workspace", self._fit_group_name)
        add_sample_log_multi.setProperty("LogNames", log_names)
        add_sample_log_multi.setProperty("LogValues", log_values)
        add_sample_log_multi.execute()

AlgorithmFactory.subscribe(IqtFitSequential)
